#include "structures.hpp"

void* malloc(int size) {
	//static char* heap_ptr = (char*)0x100000;
	static int first_call = 1;

	if (first_call) {
		// First time: set up initial block
		MemoryBlock* initial = (MemoryBlock*)0x100000; //this might need to be static, actually no ? can we just cast memblock in seperate scopes and still access further nodes?
		initial->size = (size_t)1048576 - sizeof(MemoryBlock);
		initial->available = 0;
		initial->next = NULL;
		initial->prev = NULL;
		first_call = 0;
	}

	// Now do malloc logic starting from 0x100000
	MemoryBlock* current = (MemoryBlock*)0x100000;
	while(current != NULL){//ie until the first suitable block
		//if statement to check for size ?
		if(current->available == 0 && current->size >= size + sizeof(MemoryBlock) + 1){
			//we want to return current and create a new header AFTER current that gives us access to the rest of the heap
			MemoryBlock* block = (MemoryBlock*)((char*)current + sizeof(MemoryBlock) + size);
			//basically reducing size of heap
			block->size = current->size - size - sizeof(MemoryBlock);
			block->available = 0; //its available
			block->next = current->next;
			block->prev = current;
			
			if(current->next != NULL){
				current->next->prev = block;
			}

			current->size = size;
			current->available = 1;
			current->next = block;
			return (void*)((char*)current + sizeof(MemoryBlock));
		}
		
		current = current->next;
	}

	//no suitable block found
	return NULL;
}


void free(void* memBlock){
	MemoryBlock* block = (MemoryBlock*)((char*)memBlock - sizeof(MemoryBlock));
	block->available = 0;
	// add merge logic if prev and next are also available
}


void* memcpy(void* dst, void* src, size_t n){ 
	// copies from src to dst memcpy(arr1, arr2, 10)
	// copies 10 bytes from arr1 to arr2
	unsigned char* srcBuf = (unsigned char*)src;
	unsigned char* dstBuf = (unsigned char*)dst;
	for(size_t i = 0; i < n; i++){
		dstBuf[i] = srcBuf[i];
	}
	return dst;
}


void* memset(void* dst, int c, size_t n){
	unsigned char* dstBuf = (unsigned char*)dst;
	for(size_t i = 0; i < n; i++){
		dstBuf[i] = c;
	}
	return dstBuf;
}

int memcmp(void* mem1, void* mem2){
	unsigned char* m1 = (unsigned char*)mem1;
	unsigned char* m2 = (unsigned char*)mem2;
	while(*m1 && *m2 && (*m1 == *m2)){
		m1++;
		m2++;
	}
	return (*m1 - *m2);
}


