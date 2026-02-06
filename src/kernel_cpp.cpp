#include "structures.hpp"
#include "commands.hpp"
#include "utilities.hpp"
#include "string.hpp"

extern "C" void kernel_main(){
	print(shell_prompt);
	print("Hi\n");
	print("test\n");
	print_num(-12345);
	print("\n");
	print_hex(0x198345);
}
