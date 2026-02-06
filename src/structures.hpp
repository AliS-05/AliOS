#ifndef STRUCTURES_H
#define STRUCTURES_H

#define NULL 0

typedef unsigned int uintptr_t;
typedef int boolean; 
typedef signed int intptr_t;
typedef unsigned int size_t;


struct MemoryBlock{
	size_t size;
	boolean available; // 0 = available , 1 = in use. basically a bool
	MemoryBlock* next;
	MemoryBlock* prev;
};

#endif
