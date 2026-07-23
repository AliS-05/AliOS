#ifndef PACKETHANDLING
#define PACKETHANDLING

#include <structures.h>

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

struct ethernet_header {
	uint8_t macDestination[6];
	uint8_t macSource     [6];
	uint16_t ethertype;
} __attribute__ ((packed));

struct ipv4_header {
	uint8_t version_ihl;
	uint8_t typeOfService;
	uint16_t totalLength;
	uint16_t identification;
	uint16_t flags_fragmentOffset;
	uint8_t timeToLive;
	uint8_t protocol;
	uint16_t headerChecksum;
	uint32_t sourceAddress;
	uint32_t destinationAddress;
} __attribute__((packed));

struct icmp_header {
	uint8_t type;
	uint8_t code;
	uint16_t checksum;
	uint16_t id;
	uint16_t sequence;
} __attribute__ ((packed));

struct udp_pseudo_header {
	uint32_t sourceAddr;
	uint32_t destAddr;
	uint8_t zeroes;
	uint8_t protocol;
	uint16_t udpLen;
} __attribute__ ((packed));

struct udp_header {
	uint16_t sourcePort;
	uint16_t destPort;
	uint16_t length;
	uint16_t checksum;
} __attribute__ ((packed));

void arpVectorInit(ArpVector* arpvec);
void arpVectorPush(ArpVector* arpvec, uint32_t ip, uint8_t* mac);
uint8_t* arpVectorFind(ArpVector* arpvec, uint32_t ip);
void arpVectorFree(ArpVector* arpvec);
void send_initial_arp_request();
void nic_irq_handle();
void send_arp_reply(uint8_t* senderMac, uint32_t senderIp);

uint16_t checksum(const void* data, uint32_t length);
uint16_t ipv4_checksum(struct ipv4_header* head);
uint16_t icmp_checksum(struct icmp_header* header, uint32_t length);
uint16_t udp_checksum(struct ipv4_header* ip, struct udp_header* udp, uint32_t udpLength);
void ping(uint32_t destIp);
extern uint8_t* MAC_ADDRESS;
extern uint32_t bar0;

static const uint16_t ETHERTYPE_ARP = 0x0806;
static const uint16_t ETHERTYPE_IPV4 = 0x0800;

#endif
