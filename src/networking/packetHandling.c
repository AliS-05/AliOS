#include <packetHandling.h>
#include <networking.h>
#include <utilities.h>
#include <memory.h>

#define ETH_FRAME_MAX 1518
#define ETH_FRAME_MIN 60

static uint8_t udpBuffer[ETH_FRAME_MAX];
static uint8_t transBuffer[ETH_FRAME_MAX];
ArpVector arpVector;
//vector code
void arpVectorInit(ArpVector* vec){
	vec->size = 0;
	vec->capacity = 16;
	vec->data = malloc(sizeof(ArpEntry) * vec->capacity);
}

void arpVectorPush(ArpVector* arpvec, uint32_t ip, uint8_t* mac){
	struct ArpEntry newEntry;
	newEntry.ip = ip;
	memcpy(newEntry.mac, mac, 6);
	if(arpvec->size >= arpvec->capacity){
		arpvec->capacity *= 2;
		arpvec->data = realloc(arpvec->data, sizeof(ArpEntry) * arpvec->capacity);
	}
	arpvec->data[arpvec->size++] = newEntry;
}

uint8_t* arpVectorFind(ArpVector* arpvec, uint32_t ip){
	for(int i = 0; i < arpvec->size; i++){
		if(arpvec->data[i].ip == ip){
			return arpvec->data[i].mac;
		}
	}
	return NULL;
}	

void arpVectorFree(ArpVector* arpvec){
	free(arpvec->data);
}




void send_initial_arp_request(){
	uint8_t arp_initial_request[60] = {
		// destination mac (broadcast)
		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		// source mac (filled in below)
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		// ethertype: ARP
		0x08, 0x06,
		// ARP payload
		0x00, 0x01,             // Hardware Type: Ethernet
		0x08, 0x00,             // Protocl: IPv4
		0x06,                   // Hardware length
		0x04,                   // Protocol len
		0x00, 0x01,             // op code
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // Our mac address
		10, 0, 2, 15,           // SPA: 10.0.2.15 (our ip)
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Target hardware Address: unknown, being asked for
		10, 0, 2, 1             // Target protocol address (Bridge Gateway / router)
	};

	for (int i = 0; i < 6; i++) {
		arp_initial_request[6 + i] = MAC_ADDRESS[i];   // source MAC
		arp_initial_request[22 + i] = MAC_ADDRESS[i];  // ARP sender MAC (SHA)
	}
	transmit_packet(arp_initial_request, 60);
}

void send_arp_request(uint32_t destIp){
	struct ethernet_header ethHead = {0};
	struct arp_header arpHead = {0};
	uint8_t packet[64] = {0};

	memset(ethHead.macDestination, 0xFF, 6);
	memcpy(ethHead.macSource, MAC_ADDRESS, 6);
	ethHead.ethertype = btol16(0x0806);

	arpHead.hardwareType = btol16(1);
        arpHead.protocolType = btol16(0x0800);
	arpHead.hardwareLen = 6;  
	arpHead.protocolLen = 4;
        arpHead.operation = btol16(1);
	memcpy(arpHead.senderHardwareAddress, MAC_ADDRESS, 6);
	arpHead.senderIp = this_host_ip;
	arpHead.targetIp = destIp;

	memcpy(packet, &ethHead, sizeof(struct ethernet_header));
	memcpy(packet + sizeof(struct ethernet_header), &arpHead, sizeof(struct arp_header));
	transmit_packet(packet , 60);
}

//compares subnet masks and returns gateway ip if you need routing else just returns the destIp
uint32_t next_hop(uint32_t destIp){
	if((destIp & subnet_mask) == (this_host_ip & subnet_mask)){
		return destIp;
	} else {
		return bridge_ip;
	}
}

void send_arp_reply(uint8_t* senderMac, uint32_t senderIp){
	uint8_t arp_reply[60] = {
		// destination mac (broadcast)
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		// source mac (filled in below)
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		// ethertype: ARP
		0x08, 0x06,
		// ARP payload
		0x00, 0x01,             // Hardware Type: Ethernet
		0x08, 0x00,             // Protocl: IPv4
		0x06,                   // Hardware length
		0x04,                   // Protocol len
		0x00, 0x02,             // op code
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   // Sender Hardware Address (SHA)
		10, 0, 2, 15,           // SPA: 10.0.2.15 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Target hardware Address: unknown, being asked for
		10, 0, 2, 1            // Target protocol address: 10.0.2.2 (SLIRP gateway)
	};

	for (int i = 0; i < 6; i++) {
		arp_reply[0 + i] = senderMac[i];     //destination mac address
		arp_reply[6 + i] = MAC_ADDRESS[i];   // source MAC (our mac)
		arp_reply[22 + i] = MAC_ADDRESS[i];  // sender hardware address (ours)
		arp_reply[32 + i] = senderMac[i];
		//
	}
	memcpy(&arp_reply[38], &senderIp, sizeof(uint32_t));
	transmit_packet(arp_reply, 60);

}

void start_arp_sequence(){
	init_nic();
	arpVectorInit(&arpVector);
	print_mac(MAC_ADDRESS);
	send_initial_arp_request();
}

void handle_arp(uint8_t* packetBuffer){
	uint8_t* arp = packetBuffer + 14; // skip the 14-byte Ethernet header

	uint16_t opCode = (arp[6] << 8) | arp[7];
	if(opCode == 1){ //arp reply (someone asked for our ip)
		print("ARP OPCODE 1\n");
		//arp request
		//try to fill in sender information
		uint8_t senderMac[6];
		memcpy(senderMac, arp + 8, 6); // SHA field

		uint32_t senderIp;
		memcpy(&senderIp, arp + 14, 4); // SPA field

		if(arpVectorFind(&arpVector, senderIp) == NULL){
			arpVectorPush(&arpVector, senderIp, senderMac);
		}
		//send reply packet
		send_arp_reply(senderMac, senderIp);
	}
	if (opCode == 2){ //arp discovery (were asking for someones ip)
		//reply from the switch, i think, telling us the mac address of an ip
		print("ARP OPCODE 2\n");
		//arp reply
		uint8_t senderMac[6];
		memcpy(senderMac, arp + 8, 6); // SHA field

		uint32_t senderIp;
		memcpy(&senderIp, arp + 14, 4); // SPA field

		if(arpVectorFind(&arpVector, senderIp) == NULL){
			arpVectorPush(&arpVector, senderIp, senderMac);
		}
		print("ARP ENTRY ADDED: \n");
	}
}

uint16_t checksum(const void* data, uint32_t length) {
	uint32_t sum = 0;
	const uint16_t* words = (const uint16_t*)data;

	while (length > 1) {
		sum += btol16(*words++);
		length -= 2;
	}

	if (length == 1) {
		sum += ((const uint8_t*)words)[0] << 8;
	}

	while (sum >> 16) {
		sum = (sum & 0xFFFF) + (sum >> 16);
	}

	return ltob16(~sum);
}

uint16_t ipv4_checksum(struct ipv4_header* header) {
	header->headerChecksum = 0;

	uint32_t len = (header->version_ihl & 0x0F) * 4;
	return checksum(header, len);
}

uint16_t icmp_checksum(struct icmp_header* header, uint32_t length) {
	header->checksum = 0;
	return checksum(header, length);
}

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

void send_ipv4(){
	
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


void dns_lookup(uint8_t* question, size_t qLen) {
	struct dns_message dns = {0};

	dns.transactionId = btol16(0xFFAA);

	uint16_t flags =
		(0 << 15) |	// QR
		(0 << 11) |	// Opcode
		(0 << 10) |	// AA
		(0 << 9)  |	// TC
		(1 << 8)  |	// RD
		(0 << 7)  |	// RA
		(0 << 6)  |	// Z
		(0 << 5)  |	// AD
		(0 << 4)  |	// CD
		(0);		// RCODE

	dns.flags = btol16(flags);

	dns.numQuestions = btol16(1);
	dns.numAnswers = 0;


	uint16_t qtype  = btol16(1);	// A
	uint16_t qclass = btol16(1);	// IN

	uint8_t payload[sizeof(struct dns_message) + qLen + sizeof(qtype) + sizeof(qclass)];

	uint8_t* p = payload;

	memcpy(p, &dns, sizeof(struct dns_message));
	p += sizeof(struct dns_message);

	memcpy(p, question, qLen);
	p += qLen;

	memcpy(p, &qtype, sizeof(qtype));
	p += sizeof(qtype);

	memcpy(p, &qclass, sizeof(qclass));

	send_udp(dns_server, 8080, 53, payload, sizeof(payload) );
}

void receive_dns(uint8_t* fullPacket){
	uint8_t* answers = fullPacket;
	answers += sizeof(struct ethernet_header) + sizeof(struct ipv4_header) + sizeof(struct udp_header) + sizeof(struct dns_message);
	while(*answers != 0){ //skip to null terminator of question
		answers++;
	}
	answers++; //skip null terminator itself
	answers += 4; //skip type and class
	
	uint8_t first = *answers;
	char firstBuf[first];
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
	}
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

void parse_packet(uint8_t* packetBuffer){
	//ethertypes
	uint8_t ipv4Ethertype[2] = {0x08, 0x00};
	uint8_t arpEthertype[2] = {0x08, 0x06};
	print("PARSING PACKET\n");
	if(!(memcmp(packetBuffer + 12, arpEthertype, 2))){
		print("ARP PACKET PARSED\n");
		handle_arp(packetBuffer);
	} else if(!(memcmp(packetBuffer + 12, ipv4Ethertype, 2))){
		print("IPV4 PACKET PARSED\n");
		handle_ipv4(packetBuffer);
	}
}

void nic_irq_handle(){
	//when this function is called we have just been triggered by a NIC receive timer interrupt
	//so we need to handle all packets that have been received since the previous interrupt
	read_reg(ICR); //clear interrupt
	static uint8_t rxTail = 0; 

	while(RECV_DESC_LIST[rxTail].status & (1 << 0)){ 
		struct ReceiveDescriptor* rxDesc = &RECV_DESC_LIST[rxTail];
		uint8_t* packetData = (uint8_t*)(uint32_t)rxDesc->address;

		parse_packet(packetData); 

		rxDesc->status = 0;             
		write_reg(RDT, rxTail);         
		rxTail = (rxTail + 1) % NUM_RECEIVE_DESC;
	}
}
