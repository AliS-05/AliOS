#include <networking/ethernet.h>
#include <networking/ipv4.h>
#include <networking/udp.h>
#include <networking/tftp.h>
#include <fs/fs.h>
#include <core/string.h>
#include <core/memory.h>

const int tftp_port = 69; //nice
int server_tid = 0; 

uint32_t destinationIp = 0;

uint8_t packetDataBuffer[SECTORSIZE * SECTORS_PER_CLUSTER];
int packetsRead = 0;

uint16_t prevBlock = 0;


char FILENAME[16] = {0}; //fat16 doesnt support filenames larger than 11 bytes anyways
char file[8];
char ext[3];

//function to begin the tfpt ack sequence. 
void tftp_request(uint32_t destination, char* filename, char* mode){
	size_t nameLen = strlen(filename);
	size_t modeLen = strlen(mode);

	memset(FILENAME, 0, sizeof(FILENAME));
	memcpy(FILENAME, filename, nameLen < sizeof(FILENAME) - 1 ? nameLen : sizeof(FILENAME) - 1);

	memset(file, ' ', sizeof(file));
	memset(ext,  ' ', sizeof(ext));

	size_t i = 0;
	while(FILENAME[i] != '.' && FILENAME[i] != 0){
		if(i < sizeof(file)){
			file[i] = FILENAME[i];
		}
		i++;
	}

	if(FILENAME[i] == '.'){
		i++;
		for(size_t j = 0; j < sizeof(ext) && FILENAME[i] != 0; j++, i++){
			ext[j] = FILENAME[i];
		}
	}

	destinationIp = destination;

	uint8_t payload[2 + strlen(filename) + 1 + strlen(mode) + 1];
	uint8_t* p = payload;

	*(uint16_t*)p = btol16(1);                   
	p += 2;

	memcpy(p, filename, strlen(filename) + 1);    	
	p += strlen(filename) + 1;

	memcpy(p, mode, strlen(mode) + 1); //we want octet
	p += strlen(mode) + 1;

	send_udp(destination, local_tid, tftp_port, payload, p - payload);
}

void tftp_send_ack(uint16_t blockNumber){
	uint8_t payload[4];
	struct tftp_ack_header ack;
	ack.opcode = btol16(4);
	ack.blockNum = btol16(blockNumber);
	memcpy(payload, (uint8_t*)&ack, 4);

	send_udp(destinationIp, local_tid, server_tid, payload, 4);
}

void tftp_handle_data(struct udp_header* udpHead){
	server_tid = btol16(udpHead->sourcePort);
	uint8_t* p = udpHead;
	p += sizeof(struct udp_header);
	p += 2;
	uint16_t blockNum = btol16(*(uint16_t*)p);
	p += 2;

	if(packetsRead == 4){
		writeFile(file, ext, packetDataBuffer, 2048); 
		memset(packetDataBuffer, 0, 2048);
		packetsRead = 0;
	}

	uint16_t dataLen = btol16(udpHead->length) - sizeof(struct udp_header) - 4; //4 for the data header
	memcpy(packetDataBuffer + (packetsRead * 512), p, dataLen);
	writeFile(file, ext, packetDataBuffer, packetsRead * 512 + dataLen);
	memset(packetDataBuffer, 0, sizeof(packetDataBuffer));
	packetsRead = 0;
	prevBlock = 0;

	tftp_send_ack(blockNum);
}


//so tftp functions should only really be concerned with the payload data,
//not really the packet headers or anything
void handle_tftp(struct udp_header* udpHead){
	uint8_t* p = udpHead;
	p += sizeof(struct udp_header);
	uint16_t opcode = btol16(*(uint16_t*)p);
	
	if(server_tid == 0 || opcode == 3){
		tftp_handle_data(udpHead); //however if the server tid has not been set yet i will need to know the *source* port of the incoming packet
	} else if(opcode == 4){
		print("TFTP ACK Received\n");
	} else if(opcode == 5){
		print("TFTP ERROR\n");
	}
}
