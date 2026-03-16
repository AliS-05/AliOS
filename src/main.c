#include <structures.h>
#include <commands.h>
#include <utilities.h>
#include <string.h>
#include <memory.h>
#include <io.h>
#include <ata.h>
#include <fs.h>
#include <networking.h>


extern void kernel_main(){
	init_nic();
	uint32_t* address = (uint32_t*)malloc(1025);
	print("First Malloc: ");
	print_num((uint32_t)address);

	uint32_t* address2 = (uint32_t*)aligned_malloc(1024, 64);
	print("\n\nAligned malloc: ");
	print_num((uint32_t)address2);
	print("\n");

//	uint8_t testPacket[60] = {
//		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, //Destination MAC (broadcast)
//		//source mac (my nic)
//		0x0
//	};


	print(shell_prompt);
}
