#ifndef IP4_H
#define IP4_H

#include <core/structures.h>

struct ipv4_header {
	uint8_t version_ihl;
	uint8_t typeOfService;
	uint16_t totalLength;
	uint16_t identification;
	uint16_t flags_fragmentOffset;
	uint8_t timeToLive;
	uint8_t protocol;
	uint16_t headerChecksum;
	uint32_t sourceAddress;
	uint32_t destinationAddress;
} __attribute__((packed));

uint16_t ipv4_checksum(struct ipv4_header* head);

#endif
