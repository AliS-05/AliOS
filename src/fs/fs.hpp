#ifndef FS_H
#define FS_H

#include "structures.hpp"

#define MAXFILES 12
#define SECTORSIZE 512

struct FileObject{
	char name[32];
	uint32_t startSector;
	uint32_t size;
} __attribute__((packed));

void initfs();

void listfiles();
void read_file(const char* filename);
void write_file(const char* filename, uint8_t *buffer, size_t size);
boolean delete_file(const char* filename);
#endif
