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
	struct tftp_header tHead = {0};
	size_t fileLen = strlen(filename) + 1;
	
	const char* extension = token(filename, '.');
	memcpy(file, filename, strlen(filename));
	memcpy(ext, extension, 3); // i think extension have to be 3 characters
	
	destinationIp = destination;

	uint8_t payload[2 + strlen(filename) + 1 + strlen(mode) + 1];
	uint8_t* p = payload;

	*(uint16_t*)p = btol16(1);                    
	p += 2;

	memcpy(p, filename, strlen(filename) + 1);    	
	p += strlen(filename) + 1;

	memcpy(p, mode, strlen(mode) + 1); //we want octet
	p += strlen(mode) + 1;

	send_udp(destination, local_tid, 69, payload, p - payload);
}

void tftp_send_ack(uint16_t blockNumber){
	uint8_t payload[4];
	struct tftp_ack_header ack;
	ack.opcode = 4;
	ack.blockNum = blockNumber;
	memcpy(payload, (uint8_t*)&ack, 4);

	send_udp(destinationIp, local_tid, server_tid, payload, 4);
}

//if we receive an ack we are acking a data packet
void tftp_handle_ack(struct udp_header* udpHead){
	server_tid = udpHead->sourcePort;
	uint8_t* p = udpHead;
	p += sizeof(struct udp_header);
	p += 2;
	uint16_t blockNum = (uint16_t)*p;
	p += 2;

	if(memcmp(file, 0, 16) == 0){
		print("Error with filename and tftp\n");
		return;
	}

	if(packetsRead == 4){
		writeFile(file, ext, packetDataBuffer, 2048); 
		memset(packetDataBuffer, 0, 2048);
		packetsRead = 0;
	}

	if(udpHead->length - sizeof(struct udp_header) == 512){
		//ie a full length data packet
		memcpy(packetDataBuffer + (packetsRead * 512), p, 512);
		tftp_send_ack(blockNum);
		prevBlock = blockNum;
	} else { //other wise we have our final packet lets write it to disk
		memcpy(packetDataBuffer + (packetsRead * 512), p, 512);
		writeFile(file, ext, packetDataBuffer, 2048); 
		memset(packetDataBuffer, 0, 2048);
		packetsRead = 0;
		prevBlock = 0;
	}
}


//so tftp functions should only really be concerned with the payload data,
//not really the packet headers or anything
void handle_tftp(struct udp_header* udpHead){
	if(server_tid == 0){
		tftp_handle_ack(udpHead); //however if the server tid has not been set yet i will need to know the *source* port of the incoming packet
	}
}
