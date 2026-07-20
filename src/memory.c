#include <structures.h>
#include <memory.h>

static short first_call = 1;
void* malloc(uint32_t size) {
	//static char* heap_ptr = (char*)0x100000;

	if (first_call) {
		// First time: set up initial block
		struct MemoryBlock* initial = (struct MemoryBlock*)0x100000; //this might need to be static, actually no ? can we just cast memblock in seperate scopes and still access further nodes?
		initial->size = (size_t)1048576 - sizeof(struct MemoryBlock);
		initial->available = 0;
		initial->next = NULL;
		initial->prev = NULL;
		first_call = 0;
	}

	// Now do malloc logic starting from 0x100000
	struct MemoryBlock* current = (struct MemoryBlock*)0x100000;
	while(current != NULL){//ie until the first suitable block
		//if statement to check for size ?
		if(current->available == 0 && current->size >= size + sizeof(struct MemoryBlock) + 1){
			//we want to return current and create a new header AFTER current that gives us access to the rest of the heap
			struct MemoryBlock* block = (struct MemoryBlock*)((char*)current + sizeof(struct MemoryBlock) + size);
			//basically reducing size of heap
			block->size = current->size - size - sizeof(struct MemoryBlock);
			block->available = 0; //its available
			block->next = current->next;
			block->prev = current;
			
			if(current->next != NULL){
				current->next->prev = block;
			}

			current->size = size;
			current->available = 1;
			current->next = block;
			return (void*)((char*)current + sizeof(struct MemoryBlock));
		}
		
		current = current->next;
	}

	//no suitable block found
	return NULL;
}


void free(void* memBlock){
	struct MemoryBlock* block = (struct MemoryBlock*)((char*)memBlock - sizeof(struct MemoryBlock));
	block->available = 0;
	// add merge logic if prev and next are also available
}


void* memcpy(void* dst, void* src, size_t n){ 
	// copies from src to dst memcpy(arr1, arr2, 10)
	// copies 10 bytes from arr2 to arr1
	unsigned char* srcBuf = (unsigned char*)src;
	unsigned char* dstBuf = (unsigned char*)dst;
	for(size_t i = 0; i < n; i++){
		dstBuf[i] = srcBuf[i];
	}
	return dst;
}

//sets n bytes to value of c in dst buffer
void* memset(void* dst, int c, size_t n){
	unsigned char* dstBuf = (unsigned char*)dst;
	for(size_t i = 0; i < n; i++){
		dstBuf[i] = c;
	}
	return dstBuf;
}

int memcmp(void* mem1, void* mem2, size_t n){
	unsigned char* m1 = (unsigned char*)mem1;
	unsigned char* m2 = (unsigned char*)mem2;
	for(size_t i = 0; i < n; ++i){
		if(*m1 == *m2) continue;
		else{ return (*m1 - *m2); }
	}
	return 0;
}

void* aligned_malloc(uint32_t size, uint32_t alignment){
	//this is exactly the same as malloc except i calculate the 
	struct MemoryBlock* current = (struct MemoryBlock*)0x100000;
	uint32_t aligned_addr;
	while(current != NULL){
		if(current->available == 0 && current->size >= size + alignment){
			uint32_t base_addr = (uint32_t)current + sizeof(struct MemoryBlock);
			uint32_t remainder = base_addr % alignment;
			if(remainder != 0){
				aligned_addr = base_addr + (alignment - remainder);
			} else{
				aligned_addr = base_addr;
			}
			uint32_t padding = aligned_addr - base_addr;

			// create new block for remaining heap
			struct MemoryBlock* next_block = (struct MemoryBlock*)(aligned_addr + size);
			next_block->size = current->size - size - padding - sizeof(struct MemoryBlock);
			next_block->available = 0;
			next_block->next = current->next;
			next_block->prev = current;
		if(current->next) current->next->prev = next_block;

			    current->size = size + padding;
			    current->available = 1;
			    current->next = next_block;

			    return (void*)aligned_addr;
		}
	current = current->next;
	}
	return NULL;
}

//NOTE this might break if aligned_malloc is used since metaData placement will be off
void* realloc(void* ptr, size_t newSize){
	//read MemoryBlock info from ptr - sizeof(MemoryBlock)
	//then newLocation = malloc(newSize)
	//then compare that blocks size to the new size (since realloc can reduce size not just for increasing)
	//if newSize > oldSize then memcpy(newData, oldData, oldSize)
	//else memcpy(newData, oldData, newSize)
	//[if i naively use newSize then I would write garbage bytes since its not initialized]
	//free(ptr)
	//return newLocation
	struct MemoryBlock* blockData = (struct MemoryBlock*)((uint8_t*)ptr - sizeof(struct MemoryBlock));
	size_t oldSize = blockData->size; 

	void* newLocation = malloc(newSize);

	if(newSize > oldSize){
		memcpy(newLocation, ptr, oldSize); //normal case
	} else{
		memcpy(newLocation, ptr, newSize); //truncation case
	}
	
	free(ptr);
	return newLocation;
}
