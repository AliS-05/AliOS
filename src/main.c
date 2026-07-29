#include <core/structures.h>
#include <core/commands.h>
#include <core/utilities.h>
#include <core/string.h>
#include <core/memory.h>
#include <fs/io.h>
#include <fs/ata.h>
#include <fs/fs.h>
#include <networking/networking.h>
#include <networking/packetHandling.h>
#include <fs/fat16.h>

void print_welcome_banner() {
	print("\n");
	print("  ========================================\n");
	print("             A L I   O S   v1.0          \n");
	print("  ========================================\n");
	print("\n");
	print("  - x86 Assembler\n");
	print("  - FAT16 Filesystem\n");
	print("  - Text Editor\n");
	print("  - Snake written in assembly!\n");
	print("\n");
	print("  Type 'help' to see a list of all the commands!\n");
	print("\n");
}

extern void kernel_main(){
	init_networking();
	//init_fat16_filesystem();
	print_welcome_banner();
	print(shell_prompt);

	uint8_t msg[] = "hello from alios";
        //send_udp(bridge_ip, 40000, 9999, msg, sizeof(msg) - 1);

	//uint8_t question[] = {0x06, 'g','o','o','g','l','e', 0x03, 'c','o','m', 0x00 };
	//dns_lookup(question, sizeof(question));
}
