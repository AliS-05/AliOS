#include <structures.h>
#include <commands.h>
#include <utilities.h>
#include <string.h>
#include <memory.h>
#include <io.h>
#include <ata.h>
#include <fs.h>
#include <networking.h>
#include <fat16.h>

void print_welcome_banner() {
	print("\n");
	print("  ========================================\n");
	print("             A L I   O S   v1.0          \n");
	print("  ========================================\n");
	print("\n");
	print("  * x86 Operating System from scratch\n");
	print("  * FAT16 Filesystem | Text Editor\n");
	print("  * Native x86 Assembler | Snake Game\n");
	print("\n");
	print("  Type 'help' to see available commands\n");
	print("  Type 'clear' for a clean screen\n");
	print("\n");
}

extern void kernel_main(){
	//uint8_t* mac_address = init_nic();
	//print_mac(mac_address);
	
//	uint8_t test_packet[64] = {
//		// destination mac (broadcast)
//		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
//		// source mac 
//		0x52, 0x054, 0x00, 0x12, 0x34, 0x56,
//		// ethertype 	
//		0x88, 0x88,
//		// payload (46 bytes to reach 60 minimum frame size)
//		0x68, 0x65, 0x6C, 0x6C, 0x6F, 0x00, 0x00, 0x00, //hello
//		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
//		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
//	};

	//for (int i = 0; i < 6; i++) {
	//    test_packet[6 + i] = mac_address[i];
	//}

	//transmit_packet(test_packet, 60);
	//init_fat16_filesystem();
	print_welcome_banner();
	print(shell_prompt);
}
