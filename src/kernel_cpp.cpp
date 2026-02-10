#include "structures.hpp"
#include "commands.hpp"
#include "utilities.hpp"
#include "string.hpp"
#include "memory.hpp"
#include "fs/io.hpp"
#include "fs/ata.hpp"
#include "fs/fs.hpp"

extern "C" void kernel_main(){
	//listfiles();
	//read_file("big.txt");
	
	uint8_t buffer[256];
	strcat((char*)buffer, "Testing write function");

	write_file("hi.txt", buffer, 256);
	listfiles();

	read_file("hi.txt");
}
