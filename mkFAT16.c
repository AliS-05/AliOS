// mkfs_fat16.c
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#define SECTOR_SIZE 512
#define CLUSTER_SIZE 2048  
#define FAT_TABLE_SIZE 20
#define ROOTSECTOR 41


struct BootSector{
	uint8_t jump[3];
	char oem[8];
	uint16_t bytesPerSector;
	uint8_t  sectorsPerCluster;
	uint16_t reservedSectors;
	uint8_t  fatTables;
	uint16_t rootDirectories;
	uint16_t totalSectors;
	uint8_t  mediaType;
	uint16_t numSectorsPerFat;
	uint16_t numSectorsPerTrack;
	uint16_t numHeadsPerSide;
	uint32_t numHiddenSectors;
	uint32_t largeSectorCount;
	uint8_t  driveNumber;
	uint8_t  flags; //should be reserved if not in Windows NT
	uint8_t  signature; //must be 0x28 or 0x29
	char     volumeIDSerial[4];
	char     volumeLabelString[11];
	char     systemIdentifier[8]; //see OSDevWiki /FAT#FAT_12_and_FAT_16
	uint8_t  bootCode[448];
	uint16_t bootSignature;
} __attribute__((packed));

struct File{
	char filename[8];
	char extension[3];
	uint8_t attributes;
	uint8_t reserved; // leave 0
	uint8_t creationTimeHundredths;
	uint16_t creationTime;
	uint16_t creationDate;
	uint16_t lastAccessedDate;
	uint16_t highClusterBits; //for FAT16 this is always 0
	uint16_t lastModificationTime;
	uint16_t lastModificationDate;
	uint16_t cluster;
	uint32_t fileSize;
} __attribute__((packed));

typedef struct File Directory;

FILE* disk = NULL;

void disk_open(const char* path) {
    disk = fopen(path, "w+b");   
    if (!disk) {
        perror("fopen");
    }
}

void disk_close() {
    if (disk) fclose(disk);
}

void disk_write_sector(uint32_t lba, uint8_t* buffer) {
    fseek(disk, lba * 512, SEEK_SET);
    fwrite(buffer, 512, 1, disk);
}

void disk_write_sector_count(uint32_t lba, uint8_t* buffer, uint32_t count) {
    fseek(disk, lba * 512, SEEK_SET);
    fwrite(buffer, 512, count, disk);
}

void disk_write_cluster(uint16_t cluster, uint8_t* data) {
    uint32_t lba = 73 + (cluster-2)*4; 
    fseek(disk, lba * 512, SEEK_SET);
    fwrite(data, 512, 4, disk); 
}

void disk_read_sector(uint32_t lba, uint8_t* buffer) {
    fseek(disk, lba * SECTOR_SIZE, SEEK_SET);
    fread(buffer, SECTOR_SIZE, 1, disk);
}
uint16_t findFreeCluster() { return 2; }

void initBPB(struct BootSector* b){
	memset(b, 0, sizeof(struct BootSector));


	uint8_t jmp[3] = {0xEB, 0xFE, 0x90};
	char oem[8] = "cracked!";


	memcpy(b->jump, jmp, 3);
	memcpy(b->oem, oem, 8);

	b->bytesPerSector = 512;

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
	uint16_t fat1[(SECTOR_SIZE * FAT_TABLE_SIZE) / sizeof(uint16_t)];
	uint8_t  fat1Buffer[sizeof(fat1)];

	fat1[0] = 0xFFF8; //media descriptor
	fat1[1] = 0xFFFF; //end of chain

	uint16_t fat2[(SECTOR_SIZE * FAT_TABLE_SIZE) / sizeof(uint16_t)];
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

	disk_write_sector(0, bpbBuffer);

	disk_write_sector_count(1, (uint8_t*)fat1, 20);
	disk_write_sector_count(21, (uint8_t*)fat2, 20);

	disk_write_sector_count(41, (uint8_t*)rootDirectory, 32);
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

void writeFile(char* name, uint8_t* data, uint32_t size) {
	static uint16_t nextFreeCluster = 2; // simple free cluster tracker

	uint16_t firstCluster = nextFreeCluster;
	uint16_t cluster = firstCluster;

	// write the first cluster
	disk_write_cluster(cluster, data);

	// update FAT
	uint16_t fat[FAT_TABLE_SIZE * SECTOR_SIZE / 2];
	disk_read_sector(1, (uint8_t*)fat);
	fat[0] = 0xFFF8;
	fat[1] = 0xFFFF;
	fat[cluster] = 0xFFFF;

	disk_write_sector_count(1, (uint8_t*)fat, 20);
	disk_write_sector_count(21, (uint8_t*)fat, 20);

	nextFreeCluster++; // increment for next file

	// construct File entry
	struct File file;
	memset(&file, 0, sizeof(file));

	memcpy(file.filename, name, 8);
	memcpy(file.extension, name + 8, 3);

	file.attributes = 0x20;
	file.cluster = firstCluster;
	file.fileSize = size;

	addFileRoot(&file);
}


uint8_t* read_file(const char* path, uint32_t* size_out) {
	FILE* f = fopen(path, "rb");
	if(!f) { perror("fopen"); return NULL; }
	fseek(f,0,SEEK_END);
	uint32_t size = ftell(f);
	fseek(f,0,SEEK_SET);
	uint8_t* buf = malloc(size);
	fread(buf,1,size,f);
	fclose(f);
	*size_out = size;
	return buf;
}


int main() {
	disk_open("src/fs/disk.img");
	init_fat16_filesystem();
	fflush(disk);

	uint32_t size;
	uint8_t* data;
	printf("Adding test.bin\n");
	data = read_file("./bin/test.bin", &size);
	if(data) { writeFile("TEST    BIN", data, size); free(data); }
	else{ printf("FILE NOT FOUND"); }
	
	printf("Adding snake.bin\n");
	data = read_file("./games/snake.bin", &size);
	if(data) { writeFile("SNAKE   BIN", data, size); free(data); }
	else{ printf("FILE NOT FOUND"); }
	
	printf("Adding a.bin\n");
	data = read_file("./assembler/a.bin", &size);
	if(data) { 
		writeFile("A       BIN", data, size); free(data); 
	} else{
		printf("FILE NOT FOUND - A.BIN");
	}

	// pad to 10MB
	fseek(disk, 10*1024*1024-1, SEEK_SET);
	fputc(0,disk);
	disk_close();
}
