#ifndef ATA_H
#define ATA_H
#include "./../structures.hpp"

void disk_read_sector(uint32_t lba, uint8_t* buffer);
void disk_write_sector(uint32_t lba, uint8_t* buffer);
#endif
