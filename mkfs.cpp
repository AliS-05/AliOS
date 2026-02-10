#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define SECTOR_SIZE 512
#define MAXFILES 12

struct FileEntry{
	char name[32];
	uint32_t start_sector;
	uint32_t size_bytes;
} __attribute__((packed));


int main(){
	FILE* disk = fopen("./src/fs/disk.img", "wb");
	if(!disk){
		printf("Error opening disk image");
		return 1;
	}

	struct FileEntry files[MAXFILES];
	memset(files, 0, sizeof(files));

 	// File 1: "hello.txt" at sector 1
	strcpy(files[0].name, "hello.txt");
	files[0].start_sector = 1;
	files[0].size_bytes = 13;

	// File 2: "test.txt" at sector 2
	strcpy(files[1].name, "test.txt");
	files[1].start_sector = 2;
	files[1].size_bytes = 28;

	// File 3: "big.txt" at sector 3 (spans 2 sectors)
	strcpy(files[2].name, "big.txt");
	files[2].start_sector = 3;
	files[2].size_bytes = 600;  // More than one sector

	// Write sector 0
	fwrite(files, SECTOR_SIZE, 1, disk);

	// Sector 1: hello.txt data
	char hello_data[SECTOR_SIZE] = {0};
	strcpy(hello_data, "Hello, world!");
	fwrite(hello_data, SECTOR_SIZE, 1, disk);

	// Sector 2: test.txt data
	char test_data[SECTOR_SIZE] = {0};
	strcpy(test_data, "This is a test file. Yay!");
	fwrite(test_data, SECTOR_SIZE, 1, disk);

	// Sector 3-4: big.txt data (spans 2 sectors)
	char big_data[SECTOR_SIZE * 2] = {0};
	for (int i = 0; i < 600; i++) {
		big_data[i] = 'A' + (i % 26);  // ABCDEFGH...ZABCDEFGH...
	}
	fwrite(big_data, SECTOR_SIZE, 2, disk);

	// Fill rest of disk with zeros (10MB total)
	fseek(disk, 10 * 1024 * 1024 - 1, SEEK_SET);
	fputc(0, disk);

	fclose(disk);

	printf("Created disk.img with 3 files:\n");
	printf("  hello.txt - 13 bytes at sector 1\n");
	printf("  test.txt  - 28 bytes at sector 2\n");
	printf("  big.txt   - 600 bytes at sector 3\n");

	return 0;

}

