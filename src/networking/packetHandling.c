#include <packetHandling.h>
#include <utilities.h>
#include <memory.h>
#include <networking/networking.h>
#include <networking/ethernet.h>
#include <networking/arp.h>
#include <networking/icmp.h>
#include <networking/ipv4.h>
#include <networking/udp.h>
#include <networking/dns.h>


static uint8_t udpBuffer[ETH_FRAME_MAX];
static uint8_t transBuffer[ETH_FRAME_MAX];
struct ArpVector arpVector;
struct DnsVector dnsVector;

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
