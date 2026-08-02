#ifndef ICMP_H
#define ICMP_H

#include <core/structures.h>

struct icmp_header {
	uint8_t type;
	uint8_t code;
	uint16_t checksum;
	uint16_t id;
	uint16_t sequence;
} __attribute__ ((packed));


uint16_t icmp_checksum(struct icmp_header* header, uint32_t length);
void handle_icmp(uint8_t* fullPacket);
void ping(uint32_t destIp);

#endif
