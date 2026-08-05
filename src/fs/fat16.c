#include <fs/ata.h>
#include <core/utilities.h>
#include <fs/fat16.h>
#include <core/memory.h>
#include <core/math.h>
#include <core/string.h>

#define SECTORSIZE 512 //sector size
#define NUM_TABLES 2 // 2 fat tables
#define DATA_REGION 20407 //20407 sectors, 5101 clusters
#define SECTORS_PER_CLUSTER 4
#define FAT_TABLE_SIZE 20

#define BOOTSECTOR 0
#define FAT1SECTOR 1
#define FAT2SECTOR 21
#define ROOTSECTOR 41

uint16_t fat1[(SECTORSIZE * FAT_TABLE_SIZE) / sizeof(uint16_t)];
uint16_t fat2[(SECTORSIZE * FAT_TABLE_SIZE) / sizeof(uint16_t)];


char currentDirectoryString[64];
int currentCluster = 0;

void initBPB(struct BootSector* b){
	char buf[32];
	ntos(sizeof(struct BootSector), buf, 10);
	print("\n");

	memset(b, 0, sizeof(struct BootSector));


	uint8_t jmp[3] = {0xEB, 0xFE, 0x90};
	char oem[8] = "cracked!";


	memcpy(b->jump, jmp, 3);
	memcpy(b->oem, oem, 8);

	b->bytesPerSector = 512;
	print_hex16(b->bytesPerSector);
	print("\n");

	b->sectorsPerCluster = 4;
	b->reservedSectors = 1; //boot
	b->fatTables = 2;
	b->rootDirectories = 512;
	b->totalSectors = 20480;
	b->mediaType = 0xF8;
	b->numSectorsPerFat = 20;
	b->numSectorsPerTrack = 63;
	b->numHeadsPerSide = 255;
	b->numHiddenSectors = 0;
	b->largeSectorCount = 0;

	//extended fat16
	b->driveNumber = 0x80;
	b->flags = 0xFF;
	b->signature = 0x28;
	
	char volumeID[4]     = "ALIS";
	char volumeLabel[11] = "TESTDRIVE  ";
	char systemID[8]     = "12345678";
	
	memcpy(b->volumeIDSerial, volumeID, 4);
	memcpy(b->volumeLabelString, volumeLabel, 11);
	memcpy(b->systemIdentifier, systemID, 8);

	b->bootSignature = 0xAA55;
}

void init_fat16_filesystem(){
	//boot
	struct BootSector bpb;
	initBPB(&bpb);
	uint8_t bpbBuffer[sizeof(struct BootSector)];

	memset(bpbBuffer, 0, sizeof(struct BootSector));
	memcpy(bpbBuffer, &bpb, sizeof(struct BootSector));

	//fat tables
	uint8_t  fat1Buffer[sizeof(fat1)];

	fat1[0] = 0xFFF8; //media descriptor
	fat1[1] = 0xFFFF; //end of chain

	uint8_t  fat2Buffer[sizeof(fat2)];
	fat2[0] = 0xFFF8; //media descriptor
	fat2[1] = 0xFFFF; //end of chain

	memset(&fat1[2], 0, sizeof(fat1) - 4);
	memset(&fat2[2], 0, sizeof(fat2) - 4);
	memset(fat1Buffer, 0 , sizeof(fat1Buffer));
	memset(fat2Buffer, 0 , sizeof(fat2Buffer));

	//root directory
	struct File rootDirectory[512];
	uint8_t rootDirectoryBuffer[sizeof(rootDirectory)];

	memset(rootDirectory, 0, sizeof(struct File) * 512);
	memset(rootDirectoryBuffer, 0, sizeof(rootDirectory));

	print("Writing Boot Sector...\n");
	disk_write_sector(0, bpbBuffer);
	print("Finished Writing Boot Sector!\n");

	print("Writing FAT Tables...\n");
	disk_write_sector_count(1, fat1Buffer, 20);
	disk_write_sector_count(21, fat2Buffer, 20);
	print("Finished Writing FAT Tables!\n");

	print("Writing Root Directory...\n");
	disk_write_sector_count(ROOTSECTOR, rootDirectoryBuffer, 32);
	print("Finished Writing Root Directory!\n");
}

//clusterStatus should be 0x0000 (free cluster), a 16 bit hex number for the next cluster in the chain or 0xFFFF for end of chain
void updateFatTables(uint16_t cluster, uint16_t clusterStatus){
	uint16_t fatTable[256];

	//fat 1
	disk_read_sector(FAT1SECTOR + (cluster / 256), (uint8_t*)fatTable);

	fatTable[cluster % 256] = clusterStatus;

	disk_write_sector(FAT1SECTOR + (cluster / 256), (uint8_t*)fatTable);
	
	//fat 2
	disk_read_sector(FAT2SECTOR + (cluster / 256), (uint8_t*)fatTable);

	fatTable[cluster % 256] = clusterStatus;

	disk_write_sector(FAT2SECTOR + (cluster / 256), (uint8_t*)fatTable);
	return;
}

//helper function that returns a pointer to a File struct with the name and extension passed in
int findFileRoot(const char* filename, const char* ext, struct File* file){
	struct File rootSector[16]; 
	for(uint16_t s = 0; s < 32; s++){ //reading each sector of root
		disk_read_sector(ROOTSECTOR + s, (uint8_t*)rootSector);
		for(uint32_t e = 0; e < 16; e++){ //e = entry ie file
			if(!memcmp(rootSector[e].filename, (void*)filename, 8) && !memcmp(rootSector[e].extension, (void*)ext, 3)){
				*file = rootSector[e];
				return s;
			}
		}
	}
	return -1;
}



uint16_t findFreeCluster(){
	//fat table sector = 1-21
	//fat start entry = 2, (media and metadata and stuff)
	uint16_t fatTable[SECTORSIZE / 2];


	for(int s = 1; s < 21; s++){
		disk_read_sector(s, (uint8_t*)fatTable); //read sector into mem
		for(uint16_t e = 0; e < 256; e++){
			if(fatTable[e] == 0x0000){
				fatTable[e] = 0xFFFF;
				disk_write_sector(s, (uint8_t*)fatTable);
				disk_write_sector(FAT2SECTOR + s - 1, (uint8_t*)fatTable);
				return (s - 1) * 256 + e;
			}
		}
	}
	print("NO FREE CLUSTERS FOUND\n");
	return 0; //0 is reserved so make sure not to use this ! 
}

//returns the head to a chain of clusters
uint16_t findFreeClusterCount(int count){
        uint16_t head = findFreeCluster();
        if(head == 0) return 0;
        uint16_t prev = head;
        for(int i = 1; i < count; i++){
                uint16_t cur = findFreeCluster();
                if(cur == 0){
                        print("ERROR: out of clusters\n");
                        return 0;
                }
                updateFatTables(prev, cur);      
                prev = cur;                      
        }
	return head;
}


uint16_t getNextCluster(uint16_t cluster){
	uint16_t fat[256];
	disk_read_sector(FAT1SECTOR + cluster / 256, (uint8_t *)fat);
	return fat[cluster % 256];
}

uint16_t addFileRoot(struct File* file){
	struct File rootSector[16];
	for(uint16_t s = 0; s < 32; s++){ //reading each sector of root
		disk_read_sector(ROOTSECTOR + s, (uint8_t*)rootSector);
		for(uint32_t e = 0; e < 16; e++){ //e = entry ie file
			struct File* entry = &rootSector[e];
			if(entry->filename[0] == 0x00){
				memcpy(entry, file, sizeof(struct File));
				disk_write_sector(ROOTSECTOR + s, (uint8_t*)rootSector);
				return s;
			}
		}
	}
	return 0;
}


//returns the File struct that matches filename and extension from the root table
//new function so other functions may not use this, if it aint broke dont fix it
struct File* getFileEntry(const char* filename, const char* extension){
	struct File rootDir[16];
	struct File file;
	uint32_t rootSector = findFileRoot(filename, extension, &file);
	disk_read_sector(ROOTSECTOR + rootSector, (uint8_t*)rootDir);
	for(uint32_t e = 0; e < 16; e++){ //e = entry ie file
		struct File* entry = &rootDir[e];
		if(!memcmp(file.filename, entry->filename, 8) && !memcmp(file.extension, entry->extension, 3)){
			return entry;
			break;
		}
	}
	print("File Not Found\n");
	return 0;
}




void writeFile(char* filename, char* extension, uint8_t* data, uint32_t size){
	int clustersNeeded = ceil_div(size, SECTORSIZE * SECTORS_PER_CLUSTER);
	if(clustersNeeded == 0) return;
	uint16_t headCluster = findFreeClusterCount(clustersNeeded);
	uint16_t curClust = headCluster;
	uint16_t prevClust = 0;

	struct File file;
	memset(&file, 0, sizeof(struct File));

	memcpy(file.filename, filename, 8);
	memcpy(file.extension, extension, 3);

	file.attributes = 0x20; // anything not 0x01 is writeable 
	//all time stuff is left as 0 for now as i have not implemented RTC stuff

	file.cluster = headCluster;
	file.fileSize = size;

	//now that file struct has been created i need to add the struct to
	// the root directory. so 
	//section,memcpy to buffer and write back to disk
	addFileRoot(&file);
	int currentClusterCount = 0;
	while(curClust < 0xFFF8){
		//writes cur cluster with 2048 * currentClusterCount offset into data buffer
		disk_write_cluster(curClust, data + (currentClusterCount * (SECTORSIZE * SECTORS_PER_CLUSTER)));
		curClust = getNextCluster(curClust);
		currentClusterCount++;
	}
}



//returns pointer to uint8 buffer found by findFileRoot NOT the File struct
uint8_t* readFile(const char* filename, const char* ext){
	struct File file;
	memset(&file, 0, sizeof(struct File));
	findFileRoot(filename, ext, &file);
	if(file.filename[0] == 0){
		print("Failed to find file\n");
		return NULL;
	}
	if(file.fileSize){
		//find clusters needed
		int clustersNeeded = ceil_div(file.fileSize, SECTORSIZE * SECTORS_PER_CLUSTER);
		if(clustersNeeded == 0) clustersNeeded++;
		uint8_t* fileData = (uint8_t*)malloc(clustersNeeded * SECTORSIZE * SECTORS_PER_CLUSTER);
		uint16_t curClust = file.cluster;
		int count = 0;
		while(curClust < 0xFFF8){
			disk_read_cluster(curClust, fileData + ((SECTORSIZE * SECTORS_PER_CLUSTER) * count));
			curClust = getNextCluster(curClust);
			count++;
		}
		return fileData;
	}else{
		return NULL;
	}
}



void deleteFile(const char* filename, const char* extension){
	//remove file from root directory
	struct File file;
	uint32_t rootSector = findFileRoot(filename, extension, &file);
	if(file.filename[0] == 0){ 
		print("File to delete not found\n");
		return;
	}
	
	
	struct File rootDir[16];
	//memset(rootDir, 0, SECTORSIZE);
	disk_read_sector(ROOTSECTOR + rootSector, (uint8_t*)rootDir);
	for(uint32_t e = 0; e < 16; e++){ //e = entry ie file
		struct File* entry = &rootDir[e];
		if(!memcmp(file.filename, entry->filename, 8) && !memcmp(file.extension, entry->extension, 3)){
			//0xE5 means deleted
			memset(entry->filename, 0xE5, 8);
			//delete from root sector (only file name)
			disk_write_sector(ROOTSECTOR + rootSector, (uint8_t*)rootDir);
			
			//free up clusters in fatTable 
			uint16_t curClust = file.cluster;
			uint16_t prevClust = 0;
			while(curClust < 0xFFF8){
				uint16_t next = getNextCluster(curClust);
				updateFatTables(curClust, 0x0000);
				curClust = next;
			}
			break;
		}
	}
}

//this function gives you the actual File struct so you can access attributes such as file size 
uint32_t getFileSize(const char* filename, const char* ext){
	struct File file;
	findFileRoot(filename, ext, &file);
	if(file.filename[0] == 0){
		return 0;
	}
	return file.fileSize;
}

//expects a buffer of struct File rootDir[512];
void readRoot(uint8_t* buffer){
	struct File rootSector[16]; 
	for(uint16_t s = 0; s < 32; s++){ //reading each sector of root
		disk_read_sector(ROOTSECTOR + s, (uint8_t*)rootSector);
		memcpy(buffer + (s * SECTORSIZE), (uint8_t*)rootSector, SECTORSIZE);
	}
}

void writeRoot(uint8_t* buffer){
	struct File rootSector[16]; 
	for(uint16_t s = 0; s < 32; s++){ //reading each sector of root
		disk_write_sector(ROOTSECTOR + s, buffer + (s * SECTORSIZE));
	}
}

void makeDirectory(char* dirName){
	//ok idea, if we treat each cluster as a directory, including root
	//then to make a directory we just find an empty 'slot' and create a new file in that slot. 
	//if there are no slots then the directory / cluster is full.
	// add new clusters once i get there
	print("Making Directory: ");
	print(dirName);
	print("\n");

	Directory newDir = {0};
	if(currentCluster == 0){
		int numFiles = 0;
		struct File rootDir[512];
		readRoot((uint8_t*)rootDir);
		for(uint32_t e = 0; e < 512; e++){
			struct File* entry = &rootDir[e];
			numFiles = e;
			if(entry->filename[0] == 0){
				//empty slot found lets insert our directory
				memcpy(newDir.filename, dirName, strlen(dirName));
				newDir.attributes = 0x10; //directory
				newDir.cluster = findFreeCluster();
				memcpy((uint8_t*)&rootDir[e], (uint8_t*)&newDir, sizeof(Directory));
				writeRoot((uint8_t*)rootDir);
				print("Found free cluster and wrote dir\n");
				break;
			} 
		}
		print("Current Cluster: ");
		print_hex16(currentCluster);
		uint16_t parentCluster = currentCluster;
		currentCluster = newDir.cluster;
		print("\nParent Cluster");
		print_hex16(currentCluster);
		struct File newDirectory[64];
		disk_read_cluster(currentCluster, (uint8_t*)newDirectory);

		struct File here = {0};
		here.filename[0] = '.';
		here.attributes = 0x04;
		here.cluster = currentCluster;

		struct File parent = {0};
		memcpy(parent.filename, "..", 2);
		parent.attributes = 0x04;
		parent.cluster = parentCluster;

		memcpy((uint8_t*)&newDirectory[0], (uint8_t*)&here, sizeof(struct File));
		memcpy((uint8_t*)&newDirectory[1], (uint8_t*)&parent, sizeof(struct File));
		
		disk_write_cluster(currentCluster, (uint8_t*)newDirectory);
		print("Wrote final cluster\n");
		currentCluster = parentCluster; //dont leave us in the newly created dir

	} else {
		struct File currentDirectory[64];
		disk_read_cluster(currentCluster, (uint8_t*)currentDirectory);
		for(uint32_t e = 0; e < 64; e++){
			struct File* entry = &currentDirectory[e];
			if(entry->filename[0] == 0){
				//empty slot found lets insert our directory
				memcpy(newDir.filename, dirName, 8);
				newDir.attributes = 0x10; //directory
				newDir.cluster = findFreeCluster();
				memcpy((uint8_t*)&currentDirectory[e], (uint8_t*)&newDir, sizeof(Directory));
				disk_write_cluster(currentCluster, (uint8_t*)currentDirectory);
				break;
			} 
		}
		//without error handling we have now successfully created a new directory with its own cluster. lets add '.' and '..'
		print("Current Cluster: ");
		print_hex16(currentCluster);
		uint16_t parentCluster = currentCluster;
		currentCluster = newDir.cluster;
		print("\nParent Cluster");
		print_hex16(currentCluster);
		struct File newDirectory[64];
		disk_read_cluster(currentCluster, (uint8_t*)newDirectory);

		struct File here = {0};
		here.filename[0] = '.';
		here.attributes = 0x04;
		here.cluster = currentCluster;

		struct File parent = {0};
		memcpy(parent.filename, "..", 2);
		parent.attributes = 0x04;
		parent.cluster = parentCluster;

		memcpy((uint8_t*)&newDirectory[0], (uint8_t*)&here, sizeof(struct File));
		memcpy((uint8_t*)&newDirectory[1], (uint8_t*)&parent, sizeof(struct File));

		disk_write_cluster(currentCluster, (uint8_t*)newDirectory);
		print("Wrote final cluster\n");
		currentCluster = parentCluster; //dont leave us in the newly created dir

	}


}



void changeDirectory(const char* filename){
	//cd games
	//for files in dir, if file.filename == filename curCluster = file.cluster
	print("Changing into: ");
	print(filename);
	print("\n");

	if(currentCluster == 0){
		struct File rootDir[512];
		readRoot((uint8_t*)rootDir);
		for(uint32_t e = 0; e < 512; e++){
			struct File* entry = &rootDir[e];
			if(memcmp(entry->filename, filename, strlen(filename)) == 0){
				print("CurClust was: ");
				print_hex16(currentCluster);
				currentCluster = entry->cluster;
				print("CurClust is: ");
				print_hex16(currentCluster);
			} 
		}
	} else {

		struct File curDir[64];
		disk_read_cluster(currentCluster, (uint8_t*)curDir);
		for(uint32_t e = 0; e < 64; e++){
			struct File* entry = &curDir[e];
			if(memcmp(entry->filename, filename, strlen(filename)) == 0){
				//dir found
				//i think we just need to update the current cluster and thats it
				currentCluster = entry->cluster;
				//NOTE this is the funciton that should update pwd
			}
		}
	}
}




