#include "structures.hpp"
#include "commands.hpp"
#include "utilities.hpp"
#include "memory.hpp"
#include "string.hpp"


boolean test(){
	return (' ' == 0);
}


extern "C" void kernel_main(){
	print(shell_prompt);
//	boolean wonder = test();
//	print_num((int)wonder);
//
//	char buffer[100];
//	buffer[0] = '\0'; //critical to initialize buffer
//
//	char str[] = "Hello\n";
//	print(str);
//	
//	size_t len = strlen(str);
//	print("Str len: \n");
//	print("");
//	print_num((int)len);
//	
//	print("\n");
//	strcat(buffer, "Concatted: "); //this fixes strcat not working ?
//	strcat(buffer, str);
//	print(buffer);
}
