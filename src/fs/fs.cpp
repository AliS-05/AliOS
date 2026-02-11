#include <structures.hpp>
#include <math.hpp>
#include <ata.hpp>
#include <utilities.hpp>
#include <memory.hpp>
#include <string.hpp>

#define SUCCESS true
#define FAILURE false
#define SECTORSIZE 512
#define MAXFILES 12

struct FileObject{
	char name[32];
	uint32_t startSector;
	uint32_t size;
} __attribute__((packed));


void initfs(){
	
}

size_t fileSize(const char* filename){
	uint8_t sector[SECTORSIZE];
	disk_read_sector(0, sector);
	FileObject* files = (FileObject*)sector;

	for(int i = 0; i < MAXFILES; i++){
		if(strcmp(files[i].name, filename) == 0){
			return files[i].size;
			}
	}
	return -1;
}

uint32_t calcSectorsUsed(size_t size){
	return (size + SECTORSIZE - 1) / SECTORSIZE;
}

void listfiles(){
	uint8_t sector[SECTORSIZE];
	disk_read_sector(0, sector);
	// casting sector as array of fileObjects 
	FileObject* files = (FileObject*)sector;
	print("Files: \n");
	for(int i = 0; i < MAXFILES; i++){
		print(files[i].name);
		print("\n");
	}
}

size_t read_file(const char* filename){
	uint8_t sector[SECTORSIZE];
	disk_read_sector(0, sector);
	FileObject* files = (FileObject*)sector;

	for(int i = 0; i < MAXFILES; i++){
		if(strncmp(files[i].name,filename, 32) == 0){
			print("Found file!");
			print(" Start Sector: ");
			print_num(files[i].startSector);
			print("\n");

			uint32_t start = files[i].startSector;
			size_t size = files[i].size;
			uint32_t totalSectors = calcSectorsUsed(size);

			for(uint32_t x = 0; x < totalSectors; x++){
				memset(sector, 0, SECTORSIZE);
				disk_read_sector(start + x, sector);

				for(int j = 0; j < 20; j++){
					print_hex8(sector[j]);
					print(" ");
				}

				print((char*)sector);

			}
			return 0;
		}
	}

	print("File Not Found\n");
}


void write_file(const char* filename, uint8_t* buffer, size_t size){
	//idea, read sector table, find next open sector write to that
	uint8_t sector[SECTORSIZE];
	disk_read_sector(0, sector);
	FileObject* files = (FileObject*)sector;
	uint8_t prevSector = 0;
	for(int i = 0; i < MAXFILES; i++){
		if(files[i].name[0] == 0){
			//NOTE need to implement strcpy this is not correct
			strcat(files[i].name,filename);
			files[i].startSector = ++prevSector;
			files[i].size = size;

			//updating file table
			disk_write_sector(0, sector);
			//writing to file sector location
			disk_write_sector(prevSector, buffer);
			return;
		}

		//otherwise sector wasnt found
		prevSector += calcSectorsUsed(files[i].size);
	}

	print("Error writing to file");
}

//read sector table, find startSector, calcSectorsUsed, zero out with memset() by looping, write sector table back
boolean delete_file(const char* filename){
	uint8_t sector[SECTORSIZE];
	disk_read_sector(0, sector);
	FileObject* files = (FileObject*)sector;

	for(int i = 0; i < MAXFILES; i++){
		if(strcmp(files[i].name, filename) == 0){
			
			uint8_t zeroBuffer[SECTORSIZE];
			memset(zeroBuffer, 0, SECTORSIZE);
			uint32_t secStart = files[i].startSector;
			uint32_t secs = calcSectorsUsed(files[i].size);
			
			//looping over all sectors used and writing zeroes
			//might not be strictly necessary to zero out the disk but im going to do it for now
			for(int j = 0; j < secs; j++){
				disk_write_sector(secStart + j, zeroBuffer);
			}

			strcpy(files[i].name, "0");;
			files[i].startSector = 0;
			files[i].size = 0;
			disk_write_sector(0, sector);
			return SUCCESS;
		}
	}
	return FAILURE;
}


uint8_t* cpy_file_buffer(const char* filename, uint8_t* buffer, size_t bufferSize){

	uint8_t sector[SECTORSIZE];
	disk_read_sector(0, sector);
	FileObject* files = (FileObject*)sector;

	for(uint32_t i = 0; i < MAXFILES; i++){
		if(strncmp(files[i].name,filename, 32) == 0){

			uint32_t start = files[i].startSector;
			size_t size = files[i].size;
			if(bufferSize < size) return NULL;
			//buffer not spacious enough
			uint32_t totalSectors = calcSectorsUsed(size);
			size_t bytes_copied = 0; //need this to accurately move through the buffer and not overwrite ourselves
			uint8_t curSector[SECTORSIZE];
			//looping over sectors, need to memcpy read_sector to buffer
			for(uint32_t x = 0; x < totalSectors; x++){
				disk_read_sector(start + x, curSector);
				//
				size_t bytes_to_copy = (size - bytes_copied > SECTORSIZE) ? SECTORSIZE : (size - bytes_copied);
				memcpy(buffer + bytes_copied, curSector, SECTORSIZE);
				bytes_copied += bytes_to_copy;
			}
			return buffer;
		}
	}

	print("File Not Found\n");
	return NULL;
}


