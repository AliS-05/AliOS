#include <ata.h>
#include <utilities.h>
#include <fat16.h>
#include <memory.h>

#define SECTORSIZE 512 //sector size
#define NUM_TABLES 2 // 2 fat tables
#define DATA_REGION 20407 //20407 sectors, 5101 clusters
#define SECTORS_PER_CLUSTER 4
#define FAT_TABLE_SIZE 20

void init_bpb(struct BootSector* b){
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
	init_bpb(&bpb);
	uint8_t bpbBuffer[sizeof(struct BootSector)];

	memset(bpbBuffer, 0, sizeof(struct BootSector));
	memcpy(bpbBuffer, &bpb, sizeof(struct BootSector));

	//fat tables
	uint16_t fat1[(SECTORSIZE * FAT_TABLE_SIZE) / sizeof(uint16_t)];
	fat1[0] = 0xFFF8; //media descriptor
	fat1[1] = 0xFFFF; //end of chain

	uint16_t fat2[(SECTORSIZE * FAT_TABLE_SIZE) / sizeof(uint16_t)];
	fat2[0] = 0xFFF8; //media descriptor
	fat2[1] = 0xFFFF; //end of chain

	memset(&fat1[2], 0, sizeof(fat1) - 4);
	memset(&fat2[2], 0, sizeof(fat2) - 4);

	//root directory
	struct File rootDirectory[512];
	memset(rootDirectory, 0, sizeof(struct File) * 512);
	
	print("Writing Boot Sector...\n");
	disk_write_sector(0, bpbBuffer);
	print("Finished Writing Boot Sector!\n");

	print("Writing FAT Tables...\n");
	disk_write_sector_count(1, fat1, 20);
	disk_write_sector_count(21, fat2, 20);
	print("Finished Writing FAT Tables!\n");

	print("Writing Root Directory...\n");
	disk_write_sector_count(41, rootDirectory, 32);
	print("Finished Writing Root Directory!\n");
}





