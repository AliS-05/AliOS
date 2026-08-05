#include <core/structures.h>

extern int currentCluster;

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

struct File{
	char filename[8];
	char extension[3];
	//READ_ONLY=0x01 HIDDEN=0x02 SYSTEM=0x04 VOLUME_ID=0x08 DIRECTORY=0x10 ARCHIVE=0x20
	uint8_t attributes;
	uint8_t reserved; // leave 0
	uint8_t creationTimeHundredths;
	uint16_t creationTime;
	uint16_t creationDate;
	uint16_t lastAccessedDate;
	uint16_t highClusterBits; //for FAT16 this is always 0
	uint16_t lastModificationTime;
	uint16_t lastModificationDate;
	uint16_t cluster;
	uint32_t fileSize;
} __attribute__((packed));

typedef struct File Directory;

