#include <networking/ethernet.h>
#include <networking/ipv4.h>

uint16_t ipv4_checksum(struct ipv4_header* header) {
	header->headerChecksum = 0;
	uint32_t len = (header->version_ihl & 0x0F) * 4;
	return checksum(header, len);
}

