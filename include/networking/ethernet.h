#ifndef ETHER_H
#define ETHER_H

#define ETH_FRAME_MAX 1518
#define ETH_FRAME_MIN 60

static const uint16_t ETHERTYPE_ARP = 0x0806;
static const uint16_t ETHERTYPE_IPV4 = 0x0800;
extern uint8_t* MAC_ADDRESS;
extern uint32_t bar0;

struct ethernet_header {
	uint8_t macDestination[6];
	uint8_t macSource     [6];
	uint16_t ethertype;
} __attribute__ ((packed));


uint16_t checksum(const void* data, uint32_t length);
#endif
