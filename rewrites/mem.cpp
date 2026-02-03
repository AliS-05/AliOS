





struct MemoryBlock{
	int size;
	int available;
	MemoryBlock next;
	MemoryBlock prev;
};

MemoryBlock* heap_start = (MemoryBlock*)0x100000;

void init_heap(){
	heap_start->size = 0x100000;
	heap_start->available = 0;
	heap_start->next = NULL;
	heap_start->prev = NULL;
}


void* malloc(int size){
	
	MemoryBlock* current = heap_start;

	while(current != NULL){

		if(current->available == 0 && current->size >= size + sizeof(MemoryBlock)){
			MemoryBlock* newBlock = (MemoryBlock*)((char*)current + sizeof(MemoryBlock));
			newBlock->size = current->size - size - sizeof(MemoryBlock);
			newBlock->available = 0;
			newBlock->next = current->next;
			newBlock->prev = current


			if(current->next != NULL){
				current->next->prev = newBlock;
			}
			current->size = size;
			current->available = 1;
			current->next = newBlock;
			return (void*)((char*)current + sizeof(MemoryBlock));
			
		}

		current = current->next;

	}

	return NULL;

}
