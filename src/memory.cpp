#include "structures.hpp"

void* malloc(int size) {
	//static char* heap_ptr = (char*)0x100000;
	static int first_call = 1;

	if (first_call) {
		// First time: set up initial block
		MemoryBlock* initial = (MemoryBlock*)0x100000;
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

			//new header
			MemoryBlock* block = (MemoryBlock*)((char*)current + sizeof(MemoryBlock) + size);
			block->size = current->size - size - sizeof(MemoryBlock);
			block->available = 0;
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



