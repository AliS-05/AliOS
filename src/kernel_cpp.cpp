#include "structures.hpp"
#include "commands.hpp"
#include "utilities.hpp"
#include "string.hpp"
#include "memory.hpp"

extern "C" void kernel_main(){
	print(shell_prompt);
	char* test = (char*)malloc(100);
	char* test2 = (char*)malloc(100);
	
	strcat(test, "Hello\0");
	strcat(test2, "TESTS");
	print(test);

	(char*)memcpy(test2, test, strlen(test));
	
	print("\n");

	//strcat(test2, "World");
	print(test2);
}
