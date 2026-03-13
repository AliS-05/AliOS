#include <structures.h>


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

//Packet Buffer Memory
//from 0x10000 - 0x1FFFC R/W (64KB)



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

void find_nic(){
	for(int bus = 0; bus < 256; bus++){ //256 buses
		for(int device = 0; device < 32; device++){
			uint32_t result = pci_read(bus, device, 0, 0x00);
			uint16_t vendor = result & 0xFFFF;
			uint16_t deviceID = (result >> 16) & 0xFFFF;

			if (vendor == 0x8086 && deviceID == 0x100E){
				bar0 = pci_read(bus, device, 0, 0x10);
			}
		}
	}
}

void reset_nic(){
	write_reg(CONTROL_REG, 0x04000000); //writing 32nd bit to control registers for reset
	while(read_reg(CONTROL_REG) & 0x0400000);
}

uint16_t eeprom_read(uint8_t addr) {
    write_reg(EECD, (addr << 8) | 1);
    uint32_t result;
    do {
        result = read_reg(EERD);
    } while(!(result & 0x10));
    return (result >> 16) & 0xFFFF;
}

void init_nic(){
	find_nic();
	print("FOUND NIC !!");
	reset_nic();
}
