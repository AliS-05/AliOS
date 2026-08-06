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


char currentDirectoryString[256];
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

	if(cluster < 2 || cluster >= 5101){
		print("BAD CLUSTER IN updateFatTables\n");
		return;
	}

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


char* parsePath(char* path){
	if(path[0] == '/'){
		currentCluster = 0;
		path++;
	}

	char* comp = path;
	for(char* p = path; *p; p++){
		if(*p != '/') continue;
		*p = 0;

		if(*comp){
			boolean found = false;

			if(currentCluster == 0){
				struct File sector[16];
				for(uint16_t s = 0; s < 32 && !found; s++){
					disk_read_sector(ROOTSECTOR + s, (uint8_t*)sector);
					for(int e = 0; e < 16; e++){
						if(memcmp(sector[e].filename, comp, strlen(comp)) == 0){
							currentCluster = sector[e].cluster;
							found = true;
							break;
						}
					}
				}
			} else {
				struct File dir[64];
				disk_read_cluster(currentCluster, (uint8_t*)dir);
				for(int e = 0; e < 64; e++){
					if(memcmp(dir[e].filename, comp, strlen(comp)) == 0){
						currentCluster = dir[e].cluster;
						found = true;
						break;
					}
				}
			}

			if(!found) return NULL;
		}

		comp = p + 1;
	}

	return comp;
}

int findFileRoot(const char* filename, const char* ext, struct File* file){
	struct File rootSector[16]; 
	for(uint16_t s = 0; s < 32; s++){ //reading each sector of root
		disk_read_sector(ROOTSECTOR + s, (uint8_t*)rootSector);
		for(uint32_t e = 0; e < 16; e++){ //e = entry ie file
			if(!memcmp(rootSector[e].filename, filename, 8) && (!ext || !memcmp(rootSector[e].extension, ext, 3))){
				*file = rootSector[e];
				return s;
			}
		}
	}
	return -1;
}

//helper function that returns the sector in which the desired file is stored on disk
boolean findDirRoot(const char* dirname, struct File* Directory){
	struct File rootSector[16]; 
	for(uint16_t s = 0; s < 32; s++){ //reading each sector of root
		disk_read_sector(ROOTSECTOR + s, (uint8_t*)rootSector);
		for(uint32_t e = 0; e < 16; e++){ //e = entry ie file
			if(!memcmp(rootSector[e].filename, dirname, 8)){
				memcpy((uint8_t*)Directory, (uint8_t*)&rootSector[e], sizeof(struct File));
				return true;
			}
		}
	}
	return false;
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

//pass in your file struct and it will be filled in if the file is found
//new function so other functions may not use this, if it aint broke dont fix it
//void findFileRoot(const char* filename, const char* extension, struct File* file){
//	struct File rootDir[16];
//	struct File file;
//	uint32_t rootSector = findFileRoot(filename, extension, &file);
//	disk_read_sector(ROOTSECTOR + rootSector, (uint8_t*)rootDir);
//	for(uint32_t e = 0; e < 16; e++){ //e = entry ie file
//		struct File* entry = &rootDir[e];
//		if(!memcmp(file.filename, entry->filename, 8) && !memcmp(file.extension, entry->extension, 3)){
//			memcpy((uint8_t*)file, (uint8_t*)entry, sizeof(struct File));
//			return;
//		}
//	}
//	print("File Not Found\n");
//	return 0;
//}


void addFileDirectory(struct File* file){
	struct File dir[64];
	disk_read_cluster(currentCluster, (uint8_t*)dir);
	for(uint16_t e = 0; e < 64; e++){
		struct File* entry = &dir[e];
		if(entry->filename[0] == 0){
			memcpy((uint8_t*)&dir[e], (uint8_t*)file, sizeof(struct File));
			disk_write_cluster(currentCluster, (uint8_t*)dir);
			return;
		}
	}
}


//memcpy's directory entry into your passed struct and returns true if found otherwise does nothing and returns false
boolean findFileDirectory(const char* filename, const char* extension, struct File* file){
	struct File dir[64];
	disk_read_cluster(currentCluster, (uint8_t*)dir);
	for(uint16_t e = 0; e < 64; e++){
		struct File* entry = &dir[e];
		if(!memcmp(entry->filename, filename, 8) && (!extension || !memcmp(entry->extension, extension, 3))){	
			memcpy((uint8_t*)file, (uint8_t*)entry, sizeof(struct File));
			return true;
		}
	}
	return false;
}



void writeFile(char* filename, char* extension, uint8_t* data, uint32_t size){
	int clustersNeeded = ceil_div(size, SECTORSIZE * SECTORS_PER_CLUSTER);
	if(clustersNeeded == 0) return;
	uint16_t headCluster = findFreeClusterCount(clustersNeeded);
	uint16_t curClust = headCluster;

	struct File file;
	memset(&file, 0, sizeof(struct File));

	memset(file.filename, ' ', 8);
	memset(file.extension, ' ', 3);
	for(int i = 0; i < 8 && filename[i]; i++) file.filename[i] = filename[i];
	for(int i = 0; i < 3 && extension[i]; i++) file.extension[i] = extension[i];
	file.attributes = 0x20; // anything not 0x01 is writeable 
	//all time stuff is left as 0 for now as i have not implemented RTC stuff

	file.cluster = headCluster;
	file.fileSize = size;

	//now that file struct has been created i need to add the struct to
	// the root directory. so 
	//section,memcpy to buffer and write back to disk
	if(currentCluster == 0){
		addFileRoot(&file);
	} else {
		addFileDirectory(&file);
	}
	int currentClusterCount = 0;
	while(curClust < 0xFFF8){
		//writes cur cluster with 2048 * currentClusterCount offset into data buffer
		disk_write_cluster(curClust, data + (currentClusterCount * (SECTORSIZE * SECTORS_PER_CLUSTER)));
		curClust = getNextCluster(curClust);
		currentClusterCount++;
	}
}

//expects a buffer of struct File rootDir[512];
void readRoot(uint8_t* buffer){
	struct File rootSector[16]; 
	for(uint16_t s = 0; s < 32; s++){ //reading each sector of root
		disk_read_sector(ROOTSECTOR + s, (uint8_t*)rootSector);
		memcpy(buffer + (s * SECTORSIZE), (uint8_t*)rootSector, SECTORSIZE);
	}
}

//returns pointer to uint8 buffer found by findFileRoot NOT the File struct
uint8_t* readFile(const char* filename, const char* ext){
	struct File file;
	memset(&file, 0, sizeof(struct File));
	print("READING FILE\n");
	if(currentCluster == 0){
		struct File rootDir[512];
		readRoot((uint8_t*)rootDir);
		for(uint16_t e = 0; e < 512; e++){
			struct File* entry = &rootDir[e];
			if(!(memcmp(entry->filename, filename, strlen(filename)))){
				memcpy((uint8_t*)&file, (uint8_t*)entry, sizeof(struct File));
				print("FOUDN FILE IN LOOP BREAKING\n");
				break;
			}
		}
		
	} else {
		struct File currentDirectory[64];
		disk_read_cluster(currentCluster, (uint8_t*)currentDirectory);
		for(uint32_t e = 0; e < 64; e++){
			struct File* entry = &currentDirectory[e];
			if(!(memcmp(entry->filename, filename, strlen(filename))) && !(memcmp(entry->extension, ext, strlen(ext)))){
				//file found mark as deleted
				print("FOUND FILE\n");
				memcpy((uint8_t*)&file, (uint8_t*)entry, sizeof(struct File));
				break;
			} 
		}
	}

	if(file.fileSize){
		print("FILE FOUND\n");
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
		print("FILE NOT FOUND\n");
		return NULL;
	}
}



void deleteFile(const char* filename, const char* extension){
	//remove file from root directory

	struct File file;
	memset(&file, 0, sizeof(struct File));
	if(currentCluster == 0){
		print("DELETING IN ROOT\n");
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
				return;
			}
		}
	} else {
		print("DELETING IN CUR DIR\n");
		struct File currentDirectory[64];
		disk_read_cluster(currentCluster, (uint8_t*)currentDirectory);
		for(uint32_t e = 0; e < 64; e++){
			struct File* entry = &currentDirectory[e];
			if(!(memcmp(entry->filename, filename, strlen(filename))) && (!extension ||  !(memcmp(entry->extension, extension, strlen(extension))))){
				//file found mark as deleted
				print("FOUND FILE\n");
				memset(entry->filename, 0xE5, 8);
				memcpy((uint8_t*)&currentDirectory[e], (uint8_t*)entry, sizeof(struct File));
				disk_write_cluster(currentCluster, (uint8_t*)currentDirectory);

				//free up clusters in fatTable 
				uint16_t curClust = entry->cluster;
				uint16_t prevClust = 0;
				while(curClust < 0xFFF8){
					uint16_t next = getNextCluster(curClust);
					updateFatTables(curClust, 0x0000);
					curClust = next;
				}

				return;
			} 
		}
		print("FILE NOT FOUND\n");
	}
}

//this function gives you the actual File struct so you can access attributes such as file size 
uint32_t getFileSize(const char* filename, const char* ext){
	struct File file;
	if(currentCluster == 0){
		findFileRoot(filename, ext, &file);
	} else {
		struct File currentDirectory[64];
		disk_read_cluster(currentCluster, (uint8_t*)currentDirectory);
		for(uint32_t e = 0; e < 64; e++){
			struct File* entry = &currentDirectory[e];
			if(!(memcmp(entry->filename, filename, strlen(filename))) && !(memcmp(entry->extension, ext, strlen(ext)))){
				memcpy((uint8_t*)&file, (uint8_t*)entry, sizeof(struct File));
			} 
		}
	}
	if(file.filename[0] == 0){
		return 0;
	}
	return file.fileSize;
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
		memset((uint8_t*)newDirectory, 0, sizeof(newDirectory));
		struct File here = {0};
		here.filename[0] = '.';
		here.attributes = 0x10;
		here.cluster = currentCluster;

		struct File parent = {0};
		memcpy(parent.filename, "..", 2);
		parent.attributes = 0x10;
		parent.cluster = parentCluster;

		memcpy((uint8_t*)&newDirectory[0], (uint8_t*)&here, sizeof(struct File));
		memcpy((uint8_t*)&newDirectory[1], (uint8_t*)&parent, sizeof(struct File));
		
		disk_write_cluster(currentCluster, (uint8_t*)newDirectory);
		print("Wrote final cluster\n");
		currentCluster = parentCluster; //dont leave us in the newly created dir

	} else {
		struct File currentDirectory[64];
		memset((uint8_t*)currentDirectory, 0, sizeof(currentDirectory));
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
		memset((uint8_t*)newDirectory, 0, sizeof(newDirectory));
		struct File here = {0};
		here.filename[0] = '.';
		here.attributes = 0x10;
		here.cluster = currentCluster;

		struct File parent = {0};
		memcpy(parent.filename, "..", 2);
		parent.attributes = 0x10;
		parent.cluster = parentCluster;

		memcpy((uint8_t*)&newDirectory[0], (uint8_t*)&here, sizeof(struct File));
		memcpy((uint8_t*)&newDirectory[1], (uint8_t*)&parent, sizeof(struct File));

		disk_write_cluster(currentCluster, (uint8_t*)newDirectory);
		print("Wrote final cluster\n");
		currentCluster = parentCluster; //dont leave us in the newly created dir

	}


}


void changeDirectory(const char* filename){
	struct File entry;
	boolean found = false;

	memset((uint8_t*)&entry, 0, sizeof(struct File));

	if(currentCluster == 0){
		struct File rootDir[512];
		readRoot((uint8_t*)rootDir);
		for(uint32_t e = 0; e < 512; e++){
			if(rootDir[e].filename[0] == 0x00) break;
			if((uint8_t)rootDir[e].filename[0] == 0xE5) continue;
			if(memcmp(rootDir[e].filename, filename, strlen(filename)) == 0){
				memcpy((uint8_t*)&entry, (uint8_t*)&rootDir[e], sizeof(struct File));
				found = true;
				break;
			}
		}
	} else {
		struct File curDir[64];
		disk_read_cluster(currentCluster, (uint8_t*)curDir);
		for(uint32_t e = 0; e < 64; e++){
			if(curDir[e].filename[0] == 0x00) break;
			if((uint8_t)curDir[e].filename[0] == 0xE5) continue;
			if(memcmp(curDir[e].filename, filename, strlen(filename)) == 0){
				memcpy((uint8_t*)&entry, (uint8_t*)&curDir[e], sizeof(struct File));
				found = true;
				break;
			}
		}
	}

	if(!found){
		print("Directory not found\n");
		return;
	}

	if(!(entry.attributes & 0x10) && filename[0] != '.'){
		print("Not a directory\n");
		return;
	}

	currentCluster = entry.cluster;

	if(filename[0] == '.' && filename[1] == 0){
		return;                                         
	}

	if(filename[0] == '.' && filename[1] == '.' && filename[2] == 0){
		int len = strlen(currentDirectoryString);       
		while(len > 0 && currentDirectoryString[len - 1] != '/') len--;
		if(len > 0) len--;                              
		currentDirectoryString[len] = 0;
		return;
	}

	strcat(currentDirectoryString, "/");
	strcat(currentDirectoryString, filename);
}



void printWorkingDirectory(){
		if(currentDirectoryString[0] == '\0'){
			print("/");
		} else {
			print(currentDirectoryString);
		}
}




void deleteDirectory(const char* dirname){

	if((char)dirname[0] == '.'){
		return;
	}

	struct File dir;
	uint16_t clusterSave = currentCluster;
	if(currentCluster == 0){
		if(findDirRoot(dirname, &dir)){
			//recursively free clusters
			struct File currentDirectory[64];
			currentCluster = dir.cluster;
			disk_read_cluster(dir.cluster, (uint8_t*)currentDirectory);
			for(uint32_t e = 0; e < 64; ++e){
				struct File* entry = &currentDirectory[e];
				
				if(entry->filename[0] == 0){ 
					break;
				}

				if((uint8_t)entry->filename[0] == 0xE5){
					continue;
				}

				if(entry->attributes == 0x10){
					deleteDirectory(entry->filename);
				} else {
					deleteFile(entry->filename, entry->extension);		
				}

			}
			//delete from root
			currentCluster = clusterSave; //restore cluster
			deleteFile(dirname, 0); //delete the actual directory eg 'games'
		}
		else {
			print("DIR NOT FOUND NO DELETE\n");
			return;
		}
	} else {
		if(findFileDirectory(dirname, 0, &dir)){
			//found the directory , read its cluster, then loop over each file
			// if attribute == 0x10 recursive call otherwise just deleteFile
			struct File currentDirectory[64];
			currentCluster = dir.cluster;
			disk_read_cluster(dir.cluster, (uint8_t*)currentDirectory);
			for(uint32_t e = 0; e < 64; ++e){
				struct File* entry = &currentDirectory[e];
				if(entry->filename[0] == 0){ 
					break;
				}
				if((uint8_t)entry->filename[0] == 0xE5){
					continue;
				}
				if(entry->attributes == 0x10){
					deleteDirectory(entry->filename);
				} else {
					deleteFile(entry->filename, entry->extension);		
				}

			}
			currentCluster = clusterSave;
			deleteFile(dirname, 0);
		} else {
			print("Directory not found, no delete happening\n");
		}
	}
}

