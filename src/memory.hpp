#include "structures.hpp"

#ifndef MEMORY_H
#define MEMORY_H

void* malloc(int size);
void free(void* MemoryBlock);
void* memcpy(void* dst, void* src, size_t n);

#endif
