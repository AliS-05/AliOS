#ifndef STRUCTURES_H
#define STRUCTURES_H

#define NULL 0
#define true 1
#define false 0

typedef short int boolean; 
typedef unsigned int uintptr_t;
typedef signed int intptr_t;
typedef unsigned int size_t;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long uint64_t;
//size = 4 + 4 + 4 + 4 = 16
// int + int + sizeof(ptr) + sizeof(ptr) = 16
struct MemoryBlock{
	size_t size;
	boolean available; // 0 = available , 1 = in use. basically a bool
	struct MemoryBlock* next;
	struct MemoryBlock* prev;
};

#endif
