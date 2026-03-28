#ifndef MEMORY_H
#define MEMORY_H

#include <structures.h>


void* malloc(uint32_t size);
void  free(void* MemoryBlock);
void* memcpy(void* dst, void* src, size_t n);
void* memset(void* dst,int c, size_t n);
int memcmp(void* mem1, void* mem2, size_t n);
void* aligned_malloc(uint32_t size, uint32_t alignment);

#endif
