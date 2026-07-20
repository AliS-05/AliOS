#ifndef PACKETHANDLING
#define PACKETHANDLING

#include <structures.h>

typedef struct ArpEntry{
	uint32_t ip;
	uint8_t mac[6];
} ArpEntry;

typedef struct {
	int size;
	int capacity;
	ArpEntry* data;
} ArpVector;

void arpVectorInit(ArpVector* arpvec);
void arpVectorPush(ArpVector* arpvec, uint32_t ip, uint8_t* mac);
uint8_t* arpVectorFind(ArpVector* arpvec, uint32_t ip);
void arpVectorFree(ArpVector* arpvec);
void send_initial_arp_request();
void nic_irq_handle();
extern uint8_t* MAC_ADDRESS;
extern uint32_t bar0;


#endif
