#include "structures.hpp"
#include "utilities.hpp"
#include "memory.hpp"
#include "string.hpp"

extern "C" void kernel_main(){
	

	char buffer[100];
	buffer[0] = '\0'; //critical to initialize buffer

	char str[] = "Hello\n";
	print(str);
	
	size_t len = strlen(str);
	print_num((int)len);

	strcat(buffer, "Concatted: "); //this fixes strcat not working ?
	strcat(buffer, str);
	print(buffer);
}
