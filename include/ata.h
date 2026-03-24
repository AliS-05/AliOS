#ifndef ATA_H
#define ATA_H
#include <structures.h>

void disk_read_sector(uint32_t lba, uint8_t* buffer);
void disk_read_sector_count(uint32_t lba, uint8_t* buffer, uint32_t count);


void disk_write_sector(uint32_t lba, uint8_t* buffer);
void disk_write_sector_count(uint32_t lba, uint8_t* buffer, uint32_t count);

void read_cluster(uint32_t cluter_start, uint8_t* buffer);
void write_cluster(uint32_t cluster_start, uint8_t* buffer);
#endif
