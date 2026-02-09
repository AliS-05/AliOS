#include "io.hpp"
#include "./../structures.hpp"

#define ATA_DATA       0x1F0
#define ATA_SECCOUNT   0x1F2
#define ATA_LBA_LOW    0x1F3
#define ATA_LBA_MID    0x1F4
#define ATA_LBA_HIGH   0x1F5
#define ATA_DRIVE      0x1F6
#define ATA_STATUS     0x1F7
#define ATA_COMMAND    0x1F7
#define ATA_CMD_READ   0x20

void disk_read_sector(uint32_t lba, uint8_t* buffer){
	ata_wait_busy();
	outb(ATA_DRIVE, 0xF0 | ((lba >> 24) & 0x0F)); //specifying second drive ie disk.img rather than os.bin
	outb(ATA_SECCOUNT, 1);
	outb(ATA_LBA_LOW, (uint8_t)lba);
	outb(ATA_LBA_MID, (uint8_t)(lba >> 8));
	outb(ATA_LBA_HIGH, (uint8_t)(lba >> 16));
	outb(ATA_COMMAND, ATA_CMD_READ);

	ata_wait_busy();
	ata_wait_drq();

	insw(ATA_DATA, buffer, 256);

}
