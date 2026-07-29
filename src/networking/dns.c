#include <core/utilities.h>
#include <core/memory.h>

#include <networking/ethernet.h>
#include <networking/arp.h>
#include <networking/ipv4.h>
#include <networking/icmp.h>
#include <networking/udp.h>
#include <networking/dns.h>

extern struct DnsVector dnsVector;

void dnsVectorInit(struct DnsVector* vec){
	vec->size = 0;
	vec->capacity = 16;
	vec->data = malloc(sizeof(struct DnsEntry) * vec->capacity);
}

void dnsVectorPush(struct DnsVector* vec, char* name, size_t nameLen, uint32_t ip){
	struct DnsEntry newEntry;
	newEntry.ip = ip;
	memcpy(newEntry.name, name, nameLen);
	if(vec->size >= vec->capacity){
		vec->capacity *= 2;
		vec->data = realloc(vec->data, sizeof(struct DnsEntry) * vec->capacity);
	}
	vec->data[vec->size++] = newEntry;
}

uint32_t dnsVectorFind(struct DnsVector* vec, char* name, size_t nameLen){
	for(int i = 0; i < vec->size; i++){
		if(memcmp(vec->data[i].name, name, nameLen) == 0){
			return vec->data[i].ip;
		}
	}
	return 0; //error
}	

void dnsVectorFree(struct DnsVector* vec){
	free(vec->data);
}




void ping_dns(uint8_t* question, size_t qLen){
	uint32_t destIp = dnsVectorFind(&dnsVector, question, qLen);
	print("Entering DNS LOOP\n");
	if(destIp == 0){
		for(int tries = 0; destIp == 0 && tries < 5; tries++){
			dns_lookup(question, qLen);
			for(int waits = 0; destIp == 0 && waits < 1; waits++){
				__asm__ volatile("hlt");
				destIp = dnsVectorFind(&dnsVector, question, qLen);
			}
		}
	}
	
	print("FOUND DNS IP\n");

	if(destIp == 0){
		print("Error resolving DNS\n");
		return;
	}

	ping(destIp);
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

	send_udp(dns_server, 8080, 53, payload, sizeof(payload));
}

void receive_dns(uint8_t* fullPacket){
	uint8_t* answers = fullPacket;
	uint8_t dnsStart = sizeof(struct ethernet_header) + sizeof(struct ipv4_header) + sizeof(struct udp_header);

	answers += dnsStart + sizeof(struct dns_message);
	while(*answers != 0){ //skip to null terminator of question
		answers++;
	}
	answers++; //skip null terminator itself
	answers += 4; //skip type and class
	
	//ok so answers is now at the answers section of the dns response (checked with gdb)
	// 0xC0 denotes a pointer back to the question
	if(*answers == 0xC0){
		uint8_t offset = *(answers + 1);
		uint8_t* name = fullPacket + dnsStart + offset;

		size_t nameLen = 0;                     //walk the labels to and including the null
		while(name[nameLen] != 0){
			nameLen++;
		}
		nameLen++;

		uint32_t ipAddr = *(uint32_t*)(answers + 12);

		dnsVectorPush(&dnsVector, (char*)name, nameLen, ipAddr);
		print_ip(btol32(ipAddr));
	} else {	
		print("dns response\n");
	}
}
