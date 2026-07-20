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
	print("  - x86 Assembler\n");
	print("  - FAT16 Filesystem\n");
	print("  - Text Editor\n");
	print("  - Snake written in assembly!\n");
	print("\n");
	print("  Type 'help' to see a list of all the commands!\n");
	print("\n");
}

extern void kernel_main(){


	uint8_t* mac_address = init_nic();
	print_mac(mac_address);
	
	uint8_t arp_request[60] = {
		// destination mac (broadcast)
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		// source mac (filled in below)
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		// ethertype: ARP
		0x08, 0x06,
		// ARP payload
		0x00, 0x01,             // HTYPE: Ethernet
		0x08, 0x00,             // PTYPE: IPv4
		0x06,                   // HLEN
		0x04,                   // PLEN
		0x00, 0x01,             // OPER: request
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // SHA (filled in below)
		10, 0, 2, 15,           // SPA: 10.0.2.15 (our assumed IP)
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // THA: unknown, being asked for
		10, 0, 2, 2             // TPA: 10.0.2.2 (SLIRP gateway)
		// remaining bytes auto-zero to pad to 60
	  };

	  for (int i = 0; i < 6; i++) {
		arp_request[6 + i] = mac_address[i];   // source MAC
		arp_request[22 + i] = mac_address[i];  // ARP sender MAC (SHA)
	  }


	transmit_packet(arp_request, 60);
	
	//init_fat16_filesystem();
	print_welcome_banner();
	print(shell_prompt);
}
