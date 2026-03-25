#include <io.h>
#include <structures.h>

#define ATA_DATA       0x1F0
#define ATA_SECCOUNT   0x1F2
#define ATA_LBA_LOW    0x1F3
#define ATA_LBA_MID    0x1F4
#define ATA_LBA_HIGH   0x1F5
#define ATA_DRIVE      0x1F6
#define ATA_STATUS     0x1F7
#define ATA_COMMAND    0x1F7
#define ATA_CMD_READ   0x20
#define ATA_CMD_WRITE  0x30

#define CLUSTER_SIZE 4 // ie 4 sectors
#define SUCCESS true
#define FAILURE false
#define SECTORSIZE 512
#define SECTORS_PER_CLUSTER 4


void disk_read_sector(uint32_t lba, uint8_t* buffer){
	ata_wait_busy();
	outb(ATA_DRIVE, 0xF0 | ((lba >> 24) & 0x0F)); //0xE0 = first disk 0xF0 = second disk
	outb(ATA_SECCOUNT, 1);
	outb(ATA_LBA_LOW, (uint8_t)lba);
	outb(ATA_LBA_MID, (uint8_t)(lba >> 8));
	outb(ATA_LBA_HIGH, (uint8_t)(lba >> 16));
	outb(ATA_COMMAND, ATA_CMD_READ);

	ata_wait_busy();
	ata_wait_drq();

	insw(ATA_DATA, buffer, 256);

}

void disk_read_sector_count(uint32_t lba, uint8_t* buffer, uint32_t count){
	for(uint32_t i = 0; i < count; i++){
		disk_read_sector(lba, buffer + (i * 512)); //ie size of sector
	}
			
}

void disk_write_sector(uint32_t lba, uint8_t* buffer){
	
	ata_wait_busy();
	outb(ATA_DRIVE, 0xF0 | ((lba >> 24) & 0x0F));
	outb(ATA_SECCOUNT, 1);
	outb(ATA_LBA_LOW, (uint8_t)lba);
	outb(ATA_LBA_MID, (uint8_t)(lba >> 8));
	outb(ATA_LBA_HIGH, (uint8_t)(lba >> 16));
	outb(ATA_COMMAND, ATA_CMD_WRITE); //write command

	ata_wait_busy();
	ata_wait_drq();
	
	//sending data to disk
	outsw(ATA_DATA, buffer, 256);
	outb(ATA_COMMAND, 0xE7); //cache flush command
	ata_wait_busy();
}

void disk_write_sector_count(uint32_t lba, uint8_t* buffer, uint32_t count){
	for(uint32_t i = 0; i < count; i++){
		disk_write_sector(lba, buffer + (i * 512)); //ie size of sector
	}
			
}
//clusters will be 4 sectors, 512 * 4 = 2048 
//read expects a buffer of size 2048
void disk_read_cluster(uint16_t cluster, uint8_t* buffer){
	uint32_t sector = 73 + ((cluster - 2) * 4); //data region offset
	disk_read_sector_count(sector, buffer, SECTORS_PER_CLUSTER);
}

void disk_write_cluster(uint16_t cluster, uint8_t* buffer){
	uint32_t sector = 73 + ((cluster - 2) * 4);
	disk_write_sector_count(sector, buffer, SECTORS_PER_CLUSTER);
}
