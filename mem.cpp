
char* heap_start = 0x100000;
char* heap_current = heap_start;

void* malloc(size_t size){
	void* ptr = heap_current;
	heap_current += size
	return ptr;
}

int main() {
	malloc(100);
	return 0;
}


