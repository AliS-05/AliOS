#include "structures.hpp"
#include "utilities.hpp"
#include "memory.hpp"


extern "C" void kernel_main(){
	print(shell_prompt);
	print_char('Y');
	char* msg = "YES\n";
	print(msg);
	
	print_num(-1234);

	print_hex(0xABCD098);
}
