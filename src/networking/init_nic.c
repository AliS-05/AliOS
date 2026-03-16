#include <structures.h>
#include <utilities.h>
#include <memory.h>
#include <networking.h>

#define CONTROL_REG 0x0000 //controls major operational modes for controller
#define STATUS_REG 0x0008 // "this register provides software status indication about the Ethernet controller's  settings and modes of operation
//EE Control Data
#define EECD 0x00010 //EEPROM control and data register provides simplified interface for software accesses to the eeprom
//EE Read Data
#define EERD 0x00014 //EEPROM Read Register

//Get Packets Received Count
#define GPRC 0x04074 //counts the number of good packets received of any length
//Total Packets Received
#define TPR 0x040D0 // counts total number of all packets received, might be useful
//Total Packets Transmitted
#define TPT 0x040D4 // counts total number of packets transmitted

//Receive Data FIFO Header (Register)
#define RDFH 0x02410 //stores the head of the Ethernet controller's on-chip receive data FIFO. DO NOT WRITE TO
//Receive Data FIFO Tail (Register)
#define RDFT 0x02418 //stores tail end of FIFO
//Receive Data FIFO Packet Count
#define RDFPC 0x02430 //#of receive packets currently in the FIFO

//Receive Address Low
#define RAL 0x05400
//Receive Address High
#define RAH 0x05404

//Flow Control
#define FCTRL 0x02160
#define FCAL 0x00028 //Flow control address low
#define FCAH 0x0002C //Flow control address high
#define FCT 0x00030 //Flow Control Type
#define FCTTV 0x00170 //Flow Control Transmit Timer Value

//Mulicast Table Array
#define MTA 0x05200

//Receive Registers
#define RCRTL 0x100 //control
#define RDBAL 0x2800 //base descriptor low
#define RDBAH 0x2804 // i dont think this is needed for 32 bit
#define RDLEN 0x2808 //descriptor length
#define RDH 0x2810 //descriptor head
#define RDT 0x2818 //descriptor tail

//Transmit Registers
#define	TCTL 0x400
#define TIPG 0x410
#define TDBAL 0x3800
#define TDBAH 0x3804
#define TDLEN 0x3808
#define TDH 0x3810
#define TDT 0x3818

uint32_t bar0; // base address register, add offset to talk to device at specific function port


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
				bar0 = pci_read(bus, device, 0, 0x10);
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
	static int NUM_TRANSMIT_DESC = 8; //8 descriptors
	static int TAIL = 0;
	
/*	static */ uint8_t* packetBuffer = (uint8_t*)malloc(2048 * NUM_TRANSMIT_DESC);
/*
	static */struct TransmitDescriptor* TRANS_DESC_LIST = (struct TransmitDescriptor*)malloc(sizeof(TransmitDescriptor) * NUM_TRANSMIT_DESC);
	
	for(int desc = 0; desc < NUM_TRANSMIT_DESC; desc++){
		TRANS_DESC_LIST[desc].address = (uint64_t)packetBuffer + (desc * 2048);
		TRANS_DESC_LIST[desc].length = 0;
		TRANS_DESC_LIST[desc].checksum_offset = 0;
		TRANS_DESC_LIST[desc].command = 0;
		TRANS_DESC_LIST[desc].status = 0;
		TRANS_DESC_LIST[desc].checksum_start = 0;
		TRANS_DESC_LIST[desc].special= 0;
	}

	write_reg(TDBAL, (uint32_t)TRANS_DESC_LIST); //this is physical address
	write_reg(TDBAH, 0); //zero out upper address, (32 bit addresses)
	write_reg(TDLEN, 128); //16 bytes * 8 descriptors = 128 bytes
	//software should write 0b to both head and tail
	write_reg(TDH, 0); 
	write_reg(TDT, 0);
	write_reg(TCTL, (1 << 1) | //enable bit always 1
			(1 << 3) | //Pad Short Packets
			(0x10 << 4) | //Ethernet Standard Collision Threshold
			(0x40 << 12)); //Full Duplex Collision Distance
	write_reg(TIPG, (10 << 0) | //IPGT
			(10 << 10) | //IPGR1
			(10 << 20)); //IPGR2

}

void init_nic(){

	//nic driver initialization
	boolean found_nic = find_nic();
	if(found_nic == true){
		print("FOUND NIC !!\n");
	} else{
		print("Error finding NIC\n");
	}
	reset_nic();
	enable_ASDE();
	print("Enabled ASDE\n");
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
}
