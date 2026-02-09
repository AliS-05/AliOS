#include "structures.hpp"
#include "commands.hpp"
#include "utilities.hpp"
#include "string.hpp"
#include "memory.hpp"
#include "fs/io.hpp"
#include "fs/ata.hpp"
#include "fs/fs.hpp"

extern "C" void kernel_main(){
	listfiles();
	read_file("hello.txt");
}
