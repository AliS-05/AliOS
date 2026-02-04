#include "structures.hpp"
#include "utilities.hpp"
#include "memory.hpp"


extern "C" void kernel_main(){
	print_string("First: ", 0);
	void* ptr1 = malloc(100);
	print_num((uintptr_t)ptr1);
	
	print_string("Freeing.. ", 160);
	free(ptr1);

	print_string("Second: ", 320);
	void* ptr2 = malloc(100);
	print_num((int)ptr2);

	print_string("Negative number test: ", 800);


}
