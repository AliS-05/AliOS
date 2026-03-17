#include <structures.h>
#include <utilities.h>
#include <memory.h>
#include <networking.h>

uint32_t bar0;
uint32_t NUM_TRANSMIT_DESC = 8;
uint32_t TAIL = 0;
uint8_t* transmitPacketBuffer = NULL;
struct TransmitDescriptor* TRANS_DESC_LIST = NULL;


void outl(uint16_t port, uint32_t value){
	__asm__ volatile ("outl %0, %1" : : "a"(value), "Nd"(port));
}

uint32_t inl(uint16_t port) {
	uint32_t value;
	__asm__ volatile ("inl %1, %0" : "=a"(value) : "Nd"(port));
	return value;
}

void write_reg(uint32_t offset, uint32_t value){ //
	*(volatile uint32_t*)(bar0 + offset) = value; //writes passed value to offset register
}

uint32_t read_reg(uint32_t offset){
	return *(volatile uint32_t*)(bar0 + offset); //returns value stored in offset register
}

uint32_t pci_read(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset){
	uint32_t address = (1 << 31) //enable bit always 1
                     | (bus << 16) //which bus to look on
                     | (device << 11) // which device to look for
                     | (func << 8) //which function we want
                     | (offset & 0xFC); // which register we want
	outl(0xCF8, address);
	return inl(0xCFC);
}

boolean find_nic(){
	for(int bus = 0; bus < 256; bus++){ //256 buses
		for(int device = 0; device < 32; device++){
			uint32_t result = pci_read(bus, device, 0, 0x00);
			uint16_t vendor = result & 0xFFFF;
			uint16_t deviceID = (result >> 16) & 0xFFFF;

			if (vendor == 0x8086 && deviceID == 0x100E){
				bar0 = pci_read(bus, device, 0, 0x10) & 0xFFFFFFF0;
    
			        // Enable Bus Mastering
			        uint32_t command = pci_read(bus, device, 0, 0x04);
			        print("PCI Command before: ");
			        print_hex32(command);
			        print("\n");
			        
			        command |= 0x04;  // Set bit 2 (Bus Master Enable)
			        
			        // Write back (need to use outl/inl for PCI config writes)
			        uint32_t address = (1 << 31) | (bus << 16) | (device << 11) | (0 << 8) | 0x04;
			        outl(0xCF8, address);
			        outl(0xCFC, command);
			        
			        print("PCI Command after: ");
			        print_hex32(pci_read(bus, device, 0, 0x04));
			        print("\n");
			        
			        return true;
			}
		}
	}
	return false;
}

void reset_nic(){
	write_reg(CONTROL_REG, 0x04000000); //writing 32nd bit to control registers for reset
	while(read_reg(CONTROL_REG) & 0x04000000);
}



void enable_ASDE(){ //write bit 5 to the CTRL register
	//setting this bit makes the controller automatically detect some settings
	//software must set the SLU bit for this operation, might look at 'ASD' feature

	//SLU bit - Set Link Up, 

	uint32_t value = read_reg(CONTROL_REG);
	
	value |= (0 << 3) | //LRST set to 0 enabling Auto Negotiation
		(1 << 6) | //setting SLU bit for auto negotiation
		(1 << 5) | // auto negotiation
		(0 << 30)| //VLAN mode off since we're not using vlans
		(0 << 31); //phy reset normal mode i think telling it we dont want to reset?
	write_reg(CONTROL_REG, value);
}

//we might want this later but for now just clear all the registers
//just write zero to everything for now
void disable_FCTRL(){
	uint32_t value = (0 << 31); //Reserved
	write_reg(FCTRL, value);
	write_reg(FCAH, value);
	write_reg(FCAL, value);
	write_reg(FCT, value);
	write_reg(FCTTV, value);
}

uint16_t eeprom_read(uint8_t addr) {
	write_reg(EECD, (addr << 8) | 1);
	uint32_t result;
	do {
		result = read_reg(EERD);
	} while(!(result & 0x10));
	return (result >> 16) & 0xFFFF;
}

void write_mac_address(uint32_t mac_dword, uint16_t mac_word){
	//need to write address back exactly but flip 32nd bit to set RAH AV field
	write_reg(RAL, mac_dword);
	write_reg(RAH, (mac_word | 1 << 31));
}

void read_mac_address(uint8_t* mac_address){ //mac should be a 6 byte array i think ?
//	uint16_t mac_word1 = eeprom_read(0x0); //bytes 1-2
//	uint16_t mac_word2 = eeprom_read(0x1); //bytes 2-3
//	uint16_t mac_word3 = eeprom_read(0x2); //bytes 4-5
	
	uint32_t mac_dword1 = read_reg(RAL); //first 4 bytes
	uint16_t mac_word2 = read_reg(RAH); //last 2 bytes
	
	write_mac_address(mac_dword1, mac_word2);

	//example AA:BB:CC:DD:EE:FF
	//read returns us say AA:BB
	//little endian BB:AA = 1011 1011 1010 1010
	
	//even bytes = >> 8
	//odd bytes = & 0x00FF (use python CLI !!)
	
	//low address 4 bytes math changes a bit
	mac_address[0] = (mac_dword1 & 0xFF);
	mac_address[1] = ((mac_dword1 >> 8) & 0xff);
	mac_address[2] = ((mac_dword1 >> 16) & 0xff);	
	mac_address[3] = (mac_dword1 >> 24);
	//high address stays the same
	mac_address[4] = (mac_word2 & 0xFF);
	mac_address[5] = (mac_word2 >> 8);
}

void print_mac(uint8_t* mac) {
	for(int i = 0; i < 6; i++) {
		print_hex8(mac[i]);
		if(i < 5) print(":");
	}
}

void disable_multicast(){ //will need to actually set this up in the future
	//need to write 0 to 128 registers (writes must be 32 bit)
	for(int reg = 0; reg <= 0x1FC; reg += 4){
		write_reg(MTA + reg, 0);
	}
}


//maximum packet & transmit descriptor size = 16288 bytes
void init_transmit_descriptors(){
	//everytime we receive a descriptor do
	// tail = (tail + 1) % #descriptors to update tail position
	transmitPacketBuffer = (uint8_t*)malloc(2048 * NUM_TRANSMIT_DESC);
	TRANS_DESC_LIST = (struct TransmitDescriptor*)aligned_malloc(sizeof(struct TransmitDescriptor) * NUM_TRANSMIT_DESC, 16);

	for(int desc = 0; desc < NUM_TRANSMIT_DESC; desc++){
		TRANS_DESC_LIST[desc].address = (uint64_t)transmitPacketBuffer + (desc * 2048);
		TRANS_DESC_LIST[desc].length = 0;
		TRANS_DESC_LIST[desc].checksum_offset = 0;
		TRANS_DESC_LIST[desc].command = 0;
		TRANS_DESC_LIST[desc].status = 0;
		TRANS_DESC_LIST[desc].checksum_start = 0;
		TRANS_DESC_LIST[desc].special= 0;
	}

	write_reg(TDBAL, (uint32_t)TRANS_DESC_LIST); //this is physical address

	print("Wrote TDBAL: ");
	print_hex32((uint32_t)TRANS_DESC_LIST);
	print(", read back: ");
	print_hex32(read_reg(TDBAL));
	print("\n");

	write_reg(TDBAH, 0); //zero out upper address, (32 bit addresses)
	write_reg(TDLEN, NUM_TRANSMIT_DESC * sizeof(struct TransmitDescriptor)); //16 bytes * 8 descriptors = 128 bytes
	//software should write 0b to both head and tail
	write_reg(TDH, 0); 
	write_reg(TDT, 0);
	uint32_t packets_sent = read_reg(TPT);
	print("Packets Transmitted: ");
	print_num(packets_sent);
	print("\n");
	write_reg(TCTL, (0 << 1) | //enable bit always 1 but do later?
			(1 << 3) | //Pad Short Packets
			(0x10 << 4) | //Ethernet Standard Collision Threshold
			(0x40 << 12)); //Full Duplex Collision Distance
	write_reg(TIPG, (0x10 << 0) | //IPGT
			(0x10 << 10) | //IPGR1
			(0x10 << 20)); //IPGR2
	uint32_t tctl = read_reg(TCTL);
	tctl |= (1 << 1);
	write_reg(TCTL, tctl);
	print("Transmit enabled");

}

// returns packets transmitted ?
uint16_t transmit_packet(uint8_t* packet_data, uint16_t length){
	uint32_t cur_head = read_reg(TDH);
	if(((TAIL + 1) % NUM_TRANSMIT_DESC) == cur_head){
		print("TRANSMIT RING FULL");
		return 0;
	}
	uint8_t* dest = transmitPacketBuffer + TAIL * 2048;
	memcpy(dest ,packet_data , length);

	print("\n Packet Data: ");
	for(int c = 0; c < 20; c++){
		print_hex8(dest[c]);
	}
	uint32_t old_tail = TAIL;

	print("Setting descriptor at TAIL=");

	print_num(old_tail);
	print("\n");

	TRANS_DESC_LIST[TAIL].address = (uint64_t)dest;
	print("Set address to: ");
	print_hex32((uint32_t)TRANS_DESC_LIST[TAIL].address);
	print("\n");

	TRANS_DESC_LIST[TAIL].length = length;
	print("Set length to: ");
	print_num(TRANS_DESC_LIST[TAIL].length);
	print(" (input was ");
	print_num(length);
	print(")\n");

	TRANS_DESC_LIST[TAIL].command = 0x0B;
	print("Set command to: ");
	print_hex8(TRANS_DESC_LIST[TAIL].command);
	print("\n");

	TRANS_DESC_LIST[TAIL].status = 0;


	TAIL = (TAIL + 1) % NUM_TRANSMIT_DESC;

	print("Descriptor[");
	print_num(old_tail);
	print("]:\n");
	print("  addr: ");
	print_hex32((uint32_t)TRANS_DESC_LIST[old_tail].address);
	print("\n  len: ");
	print_num(TRANS_DESC_LIST[old_tail].length);
	print("\n  cmd: ");
	print_hex8(TRANS_DESC_LIST[old_tail].command);
	print("\n");

	write_reg(TDT, TAIL);
	print("Wrote TDT = ");
	print_num(TAIL);
	print(", Read back TDT = ");
	print_num(read_reg(TDT));
	print("\n");
	return length;
}

uint8_t* init_nic(){
//nic driver initialization
	print("SIZE OF UINT64");
	print_num(sizeof(uint64_t));
	print_num(sizeof(unsigned long long));
	print(" ");
	boolean found_nic = find_nic();
	if(found_nic == true){
		print("FOUND NIC !!\n");
	} else{
		print("Error finding NIC\n");
	}
	
	print("Size of TXDESC\n");
	print_num(sizeof(struct TransmitDescriptor));

	reset_nic();
	enable_ASDE();
	print("Enabled ASDE\n");
	for (volatile int i = 0; i < 10000000; i++); // wait for link
	uint32_t status = read_reg(STATUS_REG);
	print("STATUS: ");
	print_hex32(status);
	if (status & (1 << 1)) {
		print(" - Link UP\n");
	} else {
		print(" - Link DOWN!\n");
	}

	disable_FCTRL(); //im assuming this works
	print("Disabled Flow Control Registers\n");
	//and we already disabled VLAN in enable_ASDE()

	//start receive initialization

	uint8_t* mac_address = (uint8_t*)malloc(sizeof(uint8_t) * 6);
	read_mac_address(mac_address); //modifies in place
	print_mac(mac_address);

	disable_multicast();
	print("\nDisabled MultiCast\n");

	//start receive init
	init_transmit_descriptors();


	print("Waiting for transmission...\n");
	for (volatile int i = 0; i < 10000000; i++);

	uint32_t packets_sent = read_reg(TPT);
	print("TPT after wait: ");
	print_num(packets_sent);
	print("\n");

	// Also check if descriptor status DD bit is set
	print("Descriptor status: ");
	print_hex8(TRANS_DESC_LIST[0].status);
	print("\n");


	return mac_address;
}
