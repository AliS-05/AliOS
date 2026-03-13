#include <structures.h>
#include <commands.h>
#include <utilities.h>
#include <string.h>
#include <memory.h>
#include <io.h>
#include <ata.h>
#include <fs.h>

extern void kernel_main(){
	print(shell_prompt);
}
