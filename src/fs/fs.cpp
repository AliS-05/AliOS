#include "structures.hpp"
#include "math.hpp"
#include "ata.hpp"
#include "utilities.hpp"
#include "memory.hpp"
#include "string.hpp"

#define SECTORSIZE 512
#define MAXFILES 12

struct FileObject{
	char name[32];
	uint32_t startSector;
	uint32_t size;
} __attribute__((packed));


void initfs(){
	
}

void listfiles(){
	uint8_t sector[512];
	disk_read_sector(0, sector);
	// casting sector as array of fileObjects 
	FileObject* files = (FileObject*)sector;
	print("Files: \n");
	for(int i = 0; i < MAXFILES; i++){
		print(files[i].name);
		print("\n");
	}
}

void read_file(const char* filename){
	uint8_t sector[512];
	disk_read_sector(0, sector);
	FileObject* files = (FileObject*)sector;

	for(int i = 0; i < MAXFILES; i++){
		if(strncmp(files[i].name,filename, 32) == 0){
			print("Found file!");
			print(" Start Sector: ");
			print_num(files[i].startSector);
			print("\n");
			uint32_t start = files[i].startSector;
			uint32_t size = files[i].size;
			memset(sector, 0, 512);
			disk_read_sector(start, sector);
			for(int j = 0; j < 20; j++){
				print_hex8(sector[j]);
				print(" ");
			}

			print((char*)sector);
			return;
		}
	}

	print("File Not Found\n");
}



