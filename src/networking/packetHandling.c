#include <packetHandling.h>
#include <networking.h>
#include <utilities.h>
#include <memory.h>

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
		10, 0, 2, 15,           // SPA: 10.0.2.15 
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Target hardware Address: unknown, being asked for
		10, 0, 2, 2             // Target protocol address: 10.0.2.2 (SLIRP gateway)
	};

	for (int i = 0; i < 6; i++) {
		arp_initial_request[6 + i] = MAC_ADDRESS[i];   // source MAC
		arp_initial_request[22 + i] = MAC_ADDRESS[i];  // ARP sender MAC (SHA)
	}
	transmit_packet(arp_initial_request, 60);
}

void send_arp_reply(uint8_t* senderMac, uint8_t* senderIp){
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
		10, 0, 2, 2             // Target protocol address: 10.0.2.2 (SLIRP gateway)
	};

	for (int i = 0; i < 6; i++) {
		arp_reply[0 + i] = senderMac[i];     //destination mac address
		arp_reply[6 + i] = MAC_ADDRESS[i];   // source MAC (our mac)
		arp_reply[22 + i] = MAC_ADDRESS[i];  // sender hardware address (ours)
		arp_reply[32 + i] = senderMac[i];
		arp_reply[38 + i] = senderIp[i];
		//
	}
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
	if(opCode == 1){
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
		send_arp_reply(senderMac, (uint8_t*)&senderIp);
	}
	if (opCode == 2){
		//reply from the switch i think telling us the mac address of an ip
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

uint16_t ipv4_checksum(struct ipv4_header* header){
	header->headerChecksum = 0;
	uint32_t sum = 0;
	//cast uint16_t* for summation
	uint16_t* words = (uint16_t*)header;

	uint32_t headerLength = (header->version_ihl & 0x0F) * 4;
	for (uint32_t i = 0; i < headerLength / 2; i++) {
		sum += btol16(words[i]);
	}

	while (sum >> 16) {
		sum = (sum & 0xFFFF) + (sum >> 16);
	}

	return ltol16(~sum);
}

uint16_t icmp_checksum(struct icmp_header* header, uint32_t length){
	header->checksum = 0;

	uint32_t sum = 0;
	uint16_t* words = (uint16_t*)header;

	while(length > 1){
		sum += btol16(*words++);
		length -= 2;
	}

	if(length == 1){
		sum += ((uint8_t*)words)[0] << 8;
	}

	while(sum >> 16){
		sum = (sum & 0xFFFF) + (sum >> 16);
	}

	return ltol16(~sum);
}

void send_ipv4(){

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
		uint32_t payloadLength = ipTotalLength - sizeof(struct ipv4_header) - sizeof(struct icmp_header);
		uint32_t totalPacketLength = ipTotalLength + sizeof(struct ethernet_header);
		print_num(totalPacketLength);
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
void handle_ipv4(uint8_t* packetBuffer){
	print("IPV4 PACKET\n");
	uint8_t* ipv4Packet = packetBuffer + 14; //skipping ethernet frame header
	struct ipv4_header* packet_ipv4_header = (struct ipv4_header*)ipv4Packet;
	if(packet_ipv4_header->protocol == 0x01){
		print("ICMP PACKET\n");
		handle_icmp(packetBuffer);
	} else if(packet_ipv4_header->protocol == 0x06){
		print("TCP PACKET\n");
	} else if(packet_ipv4_header->protocol == 0x11){
		print("UDP PACKET\n");
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
