#include <structures.h>

struct BootSector{
	uint8_t jump[3];
	char oem[8];
	uint16_t bytesPerSector;
	uint8_t  sectorsPerCluster;
	uint16_t reservedSectors;
	uint8_t  fatTables;
	uint16_t rootDirectories;
	uint16_t totalSectors;
	uint8_t  mediaType;
	uint16_t numSectorsPerFat;
	uint16_t numSectorsPerTrack;
	uint16_t numHeadsPerSide;
	uint32_t numHiddenSectors;
	uint32_t largeSectorCount;
	uint8_t  driveNumber;
	uint8_t  flags; //should be reserved if not in Windows NT
	uint8_t  signature; //must be 0x28 or 0x29
	char     volumeIDSerial[4];
	char     volumeLabelString[11];
	char     systemIdentifier[8]; //see OSDevWiki /FAT#FAT_12_and_FAT_16
	uint8_t  bootCode[448];
	uint16_t bootSignature;
} __attribute__((packed));


