#include <core/utilities.h>
#include <networking/ethernet.h>
#include <networking/arp.h>
#include <networking/ipv4.h>
#include <networking/udp.h>
#include <networking/dns.h>
#include <networking/networking.h>
#include <networking/tftp.h>

static uint8_t udpBuffer[ETH_FRAME_MAX];
static uint8_t transBuffer[ETH_FRAME_MAX];

extern struct ArpVector arpVector;


uint16_t udp_checksum(struct ipv4_header* ip, struct udp_header* udp, uint32_t udpLength) {
	udp->checksum = 0;

	struct udp_pseudo_header pseudo = {
		.sourceAddr = ip->sourceAddress,
		.destAddr = ip->destinationAddress,
		.zeroes = 0,
		.protocol = ip->protocol,
		.udpLen = ltob16(udpLength)
	};

	uint32_t totalLength = sizeof(struct udp_pseudo_header) + udpLength;
	if(totalLength > ETH_FRAME_MAX){
		return 0; //checksum optional in ipv4, 0 means not computed
	}

	memcpy(udpBuffer, &pseudo, sizeof(struct udp_pseudo_header));
	memcpy(udpBuffer + sizeof(struct udp_pseudo_header), udp, udpLength);

	uint16_t csum = checksum(udpBuffer, totalLength);

	//checksum of 0 is transmitted as 0xFFFF
	if(csum == 0)
		csum = 0xFFFF;

	return csum;
}

void echo_udp(uint8_t* fullPacket){
	struct ipv4_header* ipHead = (struct ipv4_header*)((uint8_t*)fullPacket + sizeof(struct ethernet_header));

	if(ipHead->destinationAddress != this_host_ip){
		return;
	}

	struct udp_header* udpHead = (struct udp_header*)((uint8_t*)ipHead + sizeof(struct ipv4_header));

	uint32_t udpLength = btol16(udpHead->length);
	uint32_t ipTotalLength = btol16(ipHead->totalLength);

	//validate before any of these numbers size a buffer or a copy
	if(udpLength < sizeof(struct udp_header) || ipTotalLength < sizeof(struct ipv4_header) + udpLength){
		print("BAD UDP LENGTH\n");
		return;
	}

	uint32_t totalPacketLength = ipTotalLength + sizeof(struct ethernet_header);
	if(totalPacketLength > ETH_FRAME_MAX){
		print("UDP FRAME TOO LARGE\n");
		return;
	}

	struct ethernet_header* ethHead = (struct ethernet_header*)fullPacket;
	uint8_t tempMac[6];
	memcpy(tempMac, ethHead->macDestination, 6);
	memcpy(ethHead->macDestination, ethHead->macSource, 6);
	memcpy(ethHead->macSource, tempMac, 6);

	uint32_t tempAddr = ipHead->sourceAddress;
	ipHead->sourceAddress = ipHead->destinationAddress;
	ipHead->destinationAddress = tempAddr;

	uint16_t tempPort = udpHead->sourcePort;
	udpHead->sourcePort = udpHead->destPort;
	udpHead->destPort = tempPort;

	uint8_t* payload = (uint8_t*)udpHead + sizeof(struct udp_header);
	uint32_t payloadLength = udpLength - sizeof(struct udp_header);

	udpHead->checksum = 0;//udp_checksum(ipHead, udpHead, udpLength);


	ipHead->headerChecksum = ipv4_checksum(ipHead);

	memset(transBuffer, 0, ETH_FRAME_MIN);
	memcpy(transBuffer, ethHead, sizeof(struct ethernet_header));
	memcpy(transBuffer + sizeof(struct ethernet_header), ipHead, sizeof(struct ipv4_header));
	memcpy(transBuffer + sizeof(struct ethernet_header) + sizeof(struct ipv4_header), udpHead, sizeof(struct udp_header));
	memcpy(transBuffer + sizeof(struct ethernet_header) + sizeof(struct ipv4_header) + sizeof(struct udp_header), payload, payloadLength);

	//ethernet minimum frame is 60 bytes, pad short echoes
	uint32_t txLength = totalPacketLength < ETH_FRAME_MIN ? ETH_FRAME_MIN : totalPacketLength;
	transmit_packet(transBuffer, txLength);
}

void send_udp(uint32_t destIp, uint16_t sourcePort, uint16_t destPort, uint8_t* payload, uint16_t payloadLength){
	if(payloadLength > 508){
		print("UDP Payload exceeds maximum safe payload size\n");
		return;
	}
	struct ethernet_header ethHead = {0};
	struct ipv4_header     ipHead  = {0};
	struct udp_header      udpHead = {0};

	uint32_t nextHop = next_hop(destIp);
	uint8_t* destMac = arpVectorFind(&arpVector, nextHop);

	for(int tries = 0; destMac == NULL && tries < 5; tries++){
		send_arp_request(nextHop);
		for(int waits = 0; destMac == NULL && waits < 5; waits++){
			__asm__ volatile("hlt");
			destMac = arpVectorFind(&arpVector, nextHop);
		}
	}

        if(destMac == NULL){
                print("ARP TIMEOUT\n");
                return;
        }

	//build ethernet header 
	memcpy(ethHead.macDestination, destMac, 6);
	memcpy(ethHead.macSource, MAC_ADDRESS, 6);
	ethHead.ethertype = btol16(0x0800);

	//build ip header
	ipHead.version_ihl = 0b01000101; //5 32 bit words, ipv4
	ipHead.typeOfService = 0; //routine packet
	ipHead.totalLength = btol16(28 + payloadLength); //ip + icmp header + payload
	ipHead.timeToLive = 128;
	ipHead.protocol = 17;
	ipHead.sourceAddress = this_host_ip;
	ipHead.destinationAddress = destIp;

	ipHead.headerChecksum = ipv4_checksum(&ipHead);
	
	//build udp header
	udpHead.sourcePort = btol16(sourcePort);
	udpHead.destPort = btol16(destPort);
	udpHead.length = btol16(payloadLength + sizeof(struct udp_header));
	udpHead.checksum = 0;
	
	uint16_t headerLengths = sizeof(struct ethernet_header) + sizeof(struct ipv4_header) + sizeof(struct udp_header);

	memset(transBuffer, 0, ETH_FRAME_MIN);
	memcpy(transBuffer, &ethHead, sizeof(struct ethernet_header));
	memcpy(transBuffer + sizeof(struct ethernet_header), &ipHead, sizeof(struct ipv4_header));
	memcpy(transBuffer + sizeof(struct ethernet_header) + sizeof(struct ipv4_header), &udpHead, sizeof(struct udp_header));
	memcpy(transBuffer + sizeof(struct ethernet_header) + sizeof(struct ipv4_header) + sizeof(struct udp_header), payload, payloadLength);
	//ethernet minimum frame is 60 bytes, pad short echoes
	transmit_packet(transBuffer, payloadLength + headerLengths);
}


void demultiplex_udp(uint8_t* fullPacket){
	struct ipv4_header* ipHead = (struct ipv4_header*)((uint8_t*)fullPacket + sizeof(struct ethernet_header));

	if(ipHead->destinationAddress != this_host_ip){
		return;
	}

	struct udp_header* udpHead = (struct udp_header*)((uint8_t*)ipHead + sizeof(struct ipv4_header));
	uint16_t sourcePort = btol16(udpHead->sourcePort);
	uint16_t destPort = btol16(udpHead->destPort);
	uint32_t udpLength = btol16(udpHead->length);
	uint32_t ipTotalLength = btol16(ipHead->totalLength);
	
	if(destPort == 7){
		print("ECHO\n");
		echo_udp(fullPacket);
	} else if(sourcePort == 53){
		receive_dns(fullPacket);
	} else if (destPort == 6900){
		// tftp port
		print("TFTP PACKET\n");
		handle_tftp(udpHead);
	}
	
}
