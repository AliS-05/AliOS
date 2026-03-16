#include <structures.h>

struct TransmitDescriptor{
	uint64_t address;
	uint16_t length;
	uint8_t checksum_offset;
	uint8_t command;
	uint8_t status; // last 4 bits are reserved
	uint8_t checksum_start;
	uint16_t special;
} TransmitDescriptor;

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
void init_nic();

