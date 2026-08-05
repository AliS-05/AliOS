#include <networking/packetHandling.h>
#include <core/utilities.h>
#include <core/memory.h>
#include <networking/networking.h>
#include <networking/ethernet.h>
#include <networking/arp.h>
#include <networking/icmp.h>
#include <networking/ipv4.h>
#include <networking/udp.h>
#include <networking/dns.h>


extern struct ArpVector arpVector;
struct DnsVector dnsVector;

void init_networking(){
	init_nic();
	arpVectorInit(&arpVector);
	dnsVectorInit(&dnsVector);
	send_initial_arp_request();
}

void handle_ipv4(uint8_t* packetBuffer){
	uint8_t* ipv4Packet = packetBuffer + 14; //skipping ethernet frame header
	struct ipv4_header* packet_ipv4_header = (struct ipv4_header*)ipv4Packet;
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
	if(!(memcmp(packetBuffer + 12, arpEthertype, 2))){
		handle_arp(packetBuffer);
	} else if(!(memcmp(packetBuffer + 12, ipv4Ethertype, 2))){
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
