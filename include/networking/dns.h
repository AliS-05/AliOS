#ifndef DNS_H
#define DNS_H

typedef struct DnsEntry {
	char* name;
	uint32_t ip;
}; 

typedef struct DnsVector {
	int size;
	int capacity;
	struct DnsEntry* data;
};


struct dns_message {
	uint16_t transactionId;
	uint16_t flags;
	uint16_t numQuestions;
	uint16_t numAnswers;
	uint16_t numAuthRR;
	uint16_t numAddRR;
} __attribute__ ((packed));


void dnsVectorInit(struct DnsVector* vec);
void dnsVectorPush(struct DnsVector* vec, char* name, size_t nameLen, uint32_t ip);
uint32_t dnsVectorFind(struct DnsVector* vec, char* name, size_t nameLen);
void dnsVectorFree(struct DnsVector* vec);

void dns_lookup();

void ping_dns(uint8_t* question, size_t qLen){
void receive_dns(uint8_t* fullPacket){


//8.8.8.8
static const uint32_t dns_server = 0x08080808;


#endif
