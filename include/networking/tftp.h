#include <core/structures.h>

//eth_header -> ipv4_header -> udp_header -> tftp
// TFTP port number = 69
//opcodes: 1 = Read, 2 = Write, 3 = Data, 4 = ACK, 5 = Error

//TFTP passes transfer idententifiers (TID's) to the UDP to be used as the soruce and dest ports

static const int local_tid = 6900;

struct tftp_header {
	uint16_t opcode;
} __attribute__((packed));

struct tftp_ack_header {
	uint16_t opcode;
	uint16_t blockNum;
} __attribute__ ((packed)) ;

void tftp_request(uint32_t destination, char* filename, char* mode);
void tftp_handle_ack(struct udp_header* u);
void handle_tftp(struct udp_header* u);
