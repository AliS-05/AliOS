#include <structures.h>

struct TransmitDescriptor{
	uint64_t address;
	uint16_t length;
	uint8_t checksum_offset;
	uint8_t command;
	uint8_t status; // last 4 bits are reserved
	uint8_t checksum_start;
	uint16_t special;
};

void outl(uint16_t port, uint32_t value);
uint32_t inl(uint16_t port);
void write_reg(uint32_t offset, uint32_t value);
uint32_t read_reg(uint32_t offset);
uint32_t pci_read(uint8_t bus, uint8_t device, uint8_t func, uint8_t offset);
boolean find_nic();
void reset_nic();
uint16_t eeprom_read(uint8_t addr);
void enable_ASDE();
void disable_FCTRL();
void print_mac(uint8_t* mac);
uint16_t transmit_packet(uint8_t* packet_data, uint16_t length);
uint8_t* init_nic();

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

extern uint32_t bar0;
extern struct TransmitDescriptor* TRANS_DESC_LIST;
extern uint32_t TAIL;
extern uint32_t NUM_TRANSMIT_DESC;
extern uint8_t* transmitPacketBuffer;
