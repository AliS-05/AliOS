#ifndef ATA_H
#define ATA_H
#include <core/structures.h>

void disk_read_sector(uint32_t lba, uint8_t* buffer);
void disk_read_sector_count(uint32_t lba, uint8_t* buffer, uint32_t count);


void disk_write_sector(uint32_t lba, uint8_t* buffer);
void disk_write_sector_count(uint32_t lba, uint8_t* buffer, uint32_t count);

void disk_read_cluster(uint16_t cluster, uint8_t* buffer);
void disk_write_cluster(uint16_t cluster, uint8_t* buffer);
#endif
