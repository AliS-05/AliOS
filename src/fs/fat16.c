#include <ata.h>
#include <utilities.h>
#include <fat16.h>
#include <memory.h>

#define SECTORSIZE 512 //sector size
#define NUM_TABLES 2 // 2 fat tables
#define DATA_REGION 20407 //20407 sectors, 5101 clusters
#define SECTORS_PER_CLUSTER 4
#define FAT_TABLE_SIZE 20

#define BOOTSECTOR 0
#define FAT1SECTOR 1
#define FAT2SECTOR 21
#define ROOTSECTOR 41

void initBPB(struct BootSector* b){
	print_num(sizeof(struct BootSector));
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
	uint16_t fat1[(SECTORSIZE * FAT_TABLE_SIZE) / sizeof(uint16_t)];
	uint8_t  fat1Buffer[sizeof(fat1)];

	fat1[0] = 0xFFF8; //media descriptor
	fat1[1] = 0xFFFF; //end of chain

	uint16_t fat2[(SECTORSIZE * FAT_TABLE_SIZE) / sizeof(uint16_t)];
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

uint16_t findFreeCluster(){
	//fat table sector = 1-21
	//fat start entry = 2, (media and metadata and stuff)
	uint16_t fatTable[SECTORSIZE / 2];


	for(int s = 1; s < 21; s++){
		disk_read_sector(s, (uint8_t*)fatTable); //read sector into mem
		for(uint16_t e = 0; e < 256; e++){
	//this doesnt actually work for multiple sectors
	// i need some way to return the sector AND the fat entry
	// bc fatentry on sector 5 is being treated as the same as fat entry
	//sector 20 right now.
			if(fatTable[e] == 0x0000){
				print("Cluster Found!\n");
				fatTable[e] = 0xFFFF;
				disk_write_sector(s, (uint8_t*)fatTable);
				disk_write_sector(FAT2SECTOR + s - 1, (uint8_t*)fatTable);
				return (s - 1) * 256 + e + 2;
			}
		}
	}
	print("NO FREE CLUSTERS FOUND\n");
	return 0; //0 is reserved so make sure not to use this !
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

//should this return the cluster where the file is written ?
void writeFile(char* filename, char* extension, uint8_t* data, uint32_t size){
	//size will be used later to calculate amount of clusters needed
	//will need to implement a findMultipleClusters

	uint16_t firstCluster = findFreeCluster();

	print("\nCluster Number: ");
	print_num(firstCluster);

	//files should be 2048 byte aligned to maximize storage otherwise this is very inefficient
	disk_write_cluster(firstCluster, data);

	

	//construct File Object. name should be absolute path

	struct File file;
	memset(&file, 0 , sizeof(struct File));
	
	memcpy(file.filename, filename, 8);
	memcpy(file.extension, extension, 3);

	file.attributes = 0x20; // anything not 0x01 is writeable 
	//all time stuff is left as 0 for now as i have not implemented RTC stuff

	file.cluster = firstCluster;
	file.fileSize = size;

	//now that file struct has been created i need to add the struct to
	// the root directory. so 
	//section,memcpy to buffer and write back to disk
	addFileRoot(&file);
}

//helper function that returns a pointer to a File struct with the name and extension passed in
boolean findFileRoot(const char* filename, const char* ext, struct File* file){
	struct File rootSector[16]; 
	for(uint16_t s = 0; s < 32; s++){ //reading each sector of root
		disk_read_sector(ROOTSECTOR + s, (uint8_t*)rootSector);
		for(uint32_t e = 0; e < 16; e++){ //e = entry ie file
			if(!memcmp(rootSector[e].filename, (void*)filename, 8) && !memcmp(rootSector[e].extension, (void*)ext, 3)){
				*file = rootSector[e];
				return true;
			}
		}
	}
	return false;
}

//returns pointer to uint8 buffer found by findFileRoot NOT the File struct
uint8_t* readFile(const char* filename, const char* ext){
	struct File file;
	memset(&file, 0, sizeof(struct File));

	if(!findFileRoot(filename, ext, &file)){
		print("Failed to find file\n");
		return NULL;
	}
	print("\nReading Cluster: ");
	print_num(file.cluster);
	if(file.fileSize){
		uint8_t* fileData = (uint8_t*)malloc(file.fileSize);
		disk_read_cluster(file.cluster, fileData);
		for(int i = 0; i < 5; i++){
			print_hex8(fileData[i]);
		}
		return fileData;
	}else{
		return NULL;
	}
}

//this function gives you the actual File struct so you can access attributes such as file size 
uint32_t getFileSize(const char* filename, const char* ext){
	struct File file;
	if(!findFileRoot(filename, ext, &file)){
		return 0;
	}
	return file.fileSize;
}

