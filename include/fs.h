#ifndef FS_H
#define FS_H

#include <structures.h>

#define MAXFILES 12
#define SECTORSIZE 512

struct FileObject{
	char name[32];
	uint32_t startSector;
	uint32_t size;
} __attribute__((packed));

void initfs();

uint32_t calcSectorsUsed(size_t size);
uint32_t fileSize(const char* filename);
void listfiles();
void listfiles_fat16();
void read_file(const char* filename);
void overwrite_file(const char* filename, uint8_t* buffer, size_t newSize);
void write_file(const char* filename, uint8_t *buffer, size_t size);
boolean delete_file(const char* filename);
uint8_t* cpy_file_buffer(const char* filename, uint8_t* buffer, size_t bufferSize);



uint16_t findFreeCluster();
uint16_t addFileRoot(struct File* file);
void writeFile(char* name, uint8_t* data, uint32_t size);
uint8_t* readFile(char* filename, char* ext);
#endif
