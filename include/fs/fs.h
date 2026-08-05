#ifndef FS_H
#define FS_H

#include <core/structures.h>

#define MAXFILES 12
#define SECTORSIZE 512
#define SECTORS_PER_CLUSTER 4


struct FileObject{
	char name[32];
	uint32_t startSector;
	uint32_t size;
} __attribute__((packed));


//flat fs stuff
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


//fat 16 stuff
uint16_t findFreeCluster();
uint16_t addFileRoot(struct File* file);
int findFileRoot(const char* filename, const char* extension, struct File* file);
void writeFile(const char* filename, const char* extension, uint8_t* data, uint32_t size);
uint8_t* readFile(const char* filename, const char* ext);
uint32_t getFileSize(const char* filename, const char* ext);
void deleteFile(const char* filename, const char* extension);
#endif
