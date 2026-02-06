#include "structures.hpp"
#include "commands.hpp"
#include "utilities.hpp"
#include "string.hpp"
#include "memory.hpp"

extern "C" void kernel_main(){
	print(shell_prompt);
	char* str = (char*)malloc(5);
	strcat(str, "12345");

}
