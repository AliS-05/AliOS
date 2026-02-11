#include <structures.hpp>
#include <commands.hpp>
#include <utilities.hpp>
#include <string.hpp>
#include <memory.hpp>
#include <io.hpp>
#include <ata.hpp>
#include <fs.hpp>

extern "C" void kernel_main(){
	print(shell_prompt);
}
