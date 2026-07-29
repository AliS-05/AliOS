#include <networking/icmp.h>

uint16_t icmp_checksum(struct icmp_header* header, uint32_t length) {
	header->checksum = 0;
	return checksum(header, length);
}

void ping(uint32_t destIp){
	struct ethernet_header ethHead = {0};
	struct ipv4_header ipHead      = {0};
	struct icmp_header icmpHead    = {0};
	uint8_t packet[64] = {0};
	
	uint32_t nextHop = next_hop(destIp);
	uint8_t* destMac = arpVectorFind(&arpVector, nextHop);
	for(int tries = 0; destMac == NULL && tries < 5; tries++){
		send_arp_request(nextHop);
		for(int waits = 0; destMac == NULL && waits < 100; waits++){
			__asm__ volatile("hlt");
			destMac = arpVectorFind(&arpVector, nextHop);
		}
	}
        if(destMac == NULL){
                print("ARP TIMEOUT\n");
                return;
        }

	print("ARP FOUND IN PING\n");
	memcpy(ethHead.macDestination, destMac, 6);
	memcpy(ethHead.macSource, MAC_ADDRESS, 6);
	ethHead.ethertype = btol16(0x0800);
				
	ipHead.version_ihl = 0b01000101; //5 32 bit words, ipv4
	ipHead.typeOfService = 0; //routine packet
	ipHead.totalLength = btol16(28); //ip + icmp header
	ipHead.timeToLive = 128;
	ipHead.protocol = 1;
	ipHead.sourceAddress = this_host_ip;
	ipHead.destinationAddress = destIp;

	ipHead.headerChecksum = ipv4_checksum(&ipHead);

	icmpHead.type = 8;
	icmpHead.code = 0;
	icmpHead.checksum = icmp_checksum(&icmpHead, sizeof(struct icmp_header));

	memcpy(packet, &ethHead, sizeof(struct ethernet_header));
	memcpy(packet + sizeof(struct ethernet_header), &ipHead, sizeof(struct ipv4_header));
	memcpy(packet + sizeof(struct ethernet_header) + sizeof(struct ipv4_header), &icmpHead, sizeof(struct icmp_header));

	transmit_packet(packet, sizeof(packet));

}

void handle_icmp(uint8_t* fullPacket){
	
	struct icmp_header* incoming_icmp_header = (struct icmp_header*)((uint8_t*)fullPacket + sizeof(struct ethernet_header) + sizeof(struct ipv4_header));

	if(incoming_icmp_header->type == 0){
		print("Echo reply received\n");
	} 
	else if(incoming_icmp_header->type == 8){
		print("Echo request received\n");
		//switch source and dest mac addrs in ethernet frame
		//switch source and dest **ip** addrs in ipv4 header
		//change icmp header type and code
		//memcpy rest of payload
		//recompute checksum
		//transmit packet

		//ethernet header
		struct ethernet_header* ethHead = (struct ethernet_header*)fullPacket;
		uint8_t tempMac[6];
		memcpy(tempMac, ethHead->macDestination, 6);
		memcpy(ethHead->macDestination, ethHead->macSource, 6);
		memcpy(ethHead->macSource, tempMac, 6);

		struct ipv4_header* ipHead = (struct ipv4_header*)((uint8_t*)fullPacket + sizeof(struct ethernet_header));
		
		uint32_t tempAddr = ipHead->sourceAddress;
		ipHead->sourceAddress = ipHead->destinationAddress;
		ipHead->destinationAddress = tempAddr;

		ipHead->headerChecksum = ipv4_checksum(ipHead);

		struct icmp_header* icmpHead = (struct icmp_header*)((uint8_t*)ipHead + sizeof(struct ipv4_header));
		icmpHead->type = 0;

		uint8_t* payload = (uint8_t*)icmpHead + sizeof(struct icmp_header);

		uint32_t ipTotalLength = btol16(ipHead->totalLength);
		if(ipTotalLength < sizeof(struct ipv4_header) + sizeof(struct icmp_header)){
			return;
		}
		uint32_t payloadLength = ipTotalLength - sizeof(struct ipv4_header) - sizeof(struct icmp_header);
		uint32_t totalPacketLength = ipTotalLength + sizeof(struct ethernet_header);
		char buf[32];
		ntos(totalPacketLength, buf, 10);
		icmpHead->checksum = icmp_checksum(icmpHead, sizeof(struct icmp_header) + payloadLength);

		uint8_t returnPacket[totalPacketLength];

		memcpy(returnPacket, ethHead, sizeof(struct ethernet_header));
		memcpy(returnPacket + sizeof(struct ethernet_header), ipHead, sizeof(struct ipv4_header));
		memcpy(returnPacket + sizeof(struct ethernet_header) + sizeof(struct ipv4_header), icmpHead, sizeof(struct icmp_header));
		memcpy(returnPacket + sizeof(struct ethernet_header) + sizeof(struct ipv4_header) + sizeof(struct icmp_header), payload, payloadLength);
		// checksum recompute + transmit_packet(returnPacket, totalPacketLength) still not written
		transmit_packet(returnPacket, totalPacketLength);
	}
}
