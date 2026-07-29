#ifndef UDP_H
#define UDP_H

struct udp_pseudo_header {
	uint32_t sourceAddr;
	uint32_t destAddr;
	uint8_t zeroes;
	uint8_t protocol;
	uint16_t udpLen;
} __attribute__ ((packed));

struct udp_header {
	uint16_t sourcePort;
	uint16_t destPort;
	uint16_t length;
	uint16_t checksum;
} __attribute__ ((packed));


uint16_t udp_checksum(struct ipv4_header* ip, struct udp_header* udp, uint32_t udpLength);

void send_udp(uint32_t destIp, uint16_t sourcePort, uint16_t destPort, uint8_t* payload, uint16_t payloadLength);


void demultiplex_udp(uint8_t* fullPacket);

void echo_udp(uint8_t* fullPacket);

#endif
