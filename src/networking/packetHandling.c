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
	read_reg(ICR);
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
		0x00, 0x01,             // op code
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

void parse_packet(uint8_t* packetBuffer){
	uint8_t* arp = packetBuffer + 14; // skip the 14-byte Ethernet header

	uint16_t opCode = (arp[6] << 8) | arp[7];
	if(opCode == 1){
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
	if (opCode == 2){
		//arp reply
		uint8_t senderMac[6];
		memcpy(senderMac, arp + 8, 6); // SHA field

		uint32_t senderIp;
		memcpy(&senderIp, arp + 14, 4); // SPA field

		if(arpVectorFind(&arpVector, senderIp) == NULL){
			arpVectorPush(&arpVector, senderIp, senderMac);
		}
	}
}

void nic_irq_handle(){
	//when this function is called we have just been triggered by a NIC receive timer interrupt
	//so we need to handle all packets that have been received since the previous interrupt
	for(int i = 0; i < NUM_RECEIVE_DESC; i++){
		struct ReceiveDescriptor* rxDesc = &RECV_DESC_LIST[i];
		if(rxDesc->status == 0){
			continue; //continue because later descriptors may be none null ?
		} else {
		      	uint8_t* packetData = (uint8_t*)(uint32_t)rxDesc->address;
		      	parse_packet(packetData);
		      	rxDesc->status = 0;
		      	rxDesc->status = 0;
		      	write_reg(RDT, i);
          	}
	}
}

