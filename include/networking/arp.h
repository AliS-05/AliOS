#ifndef ARP_H
#define ARP_H

//10.0.2.15
static const uint32_t this_host_ip = 0x0F02000A;
//10.0.2.1
static const uint32_t bridge_ip = 0x0102000A;
//255.255.255.0
static const uint32_t subnet_mask = 0x00FFFFFF;

typedef struct ArpEntry{
	uint32_t ip;
	uint8_t mac[6];
} ArpEntry;

typedef struct ArpVector {
	int size;
	int capacity;
	ArpEntry* data;
} ArpVector;

struct arp_header {
	uint16_t hardwareType;
	uint16_t protocolType;
	uint8_t  hardwareLen;
	uint8_t  protocolLen;
	uint16_t operation;
	uint8_t  senderHardwareAddress[6]; //mac
	uint32_t senderIp;
	uint8_t  targetHardwareAddress[6];
	uint32_t targetIp;
}__attribute__((packed));


void arpVectorInit(ArpVector* arpvec);
void arpVectorPush(ArpVector* arpvec, uint32_t ip, uint8_t* mac);
uint8_t* arpVectorFind(ArpVector* arpvec, uint32_t ip);
void arpVectorFree(ArpVector* arpvec);

void send_initial_arp_request();
void send_arp_reply(uint8_t* senderMac, uint32_t senderIp);

#endif
