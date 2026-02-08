#ifndef FAT16_H
#define FAT16_H

#include "utilities.hpp"

struct fat16_bpb{
	uint8_t BS_jmpBoot[3];
	char oemName[8];
	size_t bytesPerSector = 1024;
	unsigned char secPerClus = 16;
	size_t resvdSecCnt = 2;
	unsigned char numFATs = 2;
	uint16_t totSec16 = /*idk*/;
	uint8_t media = 0xF8;
	uint16_t FATSz16 =  /*idk*/;
	uint16_t secPerTrack = /*idk*/;
	uint16_t numHeads =  /*idk*/;
	int hiddenSectors = 0;
	int totSec32 = 0;
	uint8_t drvNum = 0x00;
	uint8_t reserved = 0x0;
	uint8_t bootSig = 0x29;
	int volID = /*some number*/;
	char volLab[11];/*i think this is just C: or D: ?*/;
	char fileSysType[8];
	// then 448 bytes of 0x00
	int signature = 0x55AA
	// fill with zeroes


} __attribute__((packed));

#endif
