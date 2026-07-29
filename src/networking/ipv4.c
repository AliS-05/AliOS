#include <networking/ipv4.h>

uint16_t ipv4_checksum(struct ipv4_header* header) {
	header->headerChecksum = 0;

	uint32_t len = (header->version_ihl & 0x0F) * 4;
	return checksum(header, len);
}


void handle_ipv4(uint8_t* packetBuffer){
	print("IPV4 PACKET\n");
	uint8_t* ipv4Packet = packetBuffer + 14; //skipping ethernet frame header
	struct ipv4_header* packet_ipv4_header = (struct ipv5_header*)ipv4Packet;
	if(packet_ipv4_header->protocol == 0x01){
		print("ICMP PACKET\n");
		handle_icmp(packetBuffer);
	} else if(packet_ipv4_header->protocol == 0x06){
		print("TCP PACKET\n");
	} else if(packet_ipv4_header->protocol == 0x11){
		demultiplex_udp(packetBuffer);
	} else {
		print("IPV4 PROTOCOL NOT RECOGNIZED\n");
	}
}
