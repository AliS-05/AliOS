#include <structures.h>
#include <math.h>
#include <ata.h>
#include <utilities.h>
#include <memory.h>
#include <string.h>
#include <fat16.h>

#define SUCCESS true
#define FAILURE false
#define SECTORSIZE 512
#define MAXFILES 12
#define ROOTSECTOR 41
#define CLUSTERSIZE 2048

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
	struct FileObject* files = (struct FileObject*)sector;

	for(int i = 0; i < MAXFILES; i++){
		if(strcmp(files[i].name, filename) == 0){
			return files[i].size;
			}
	}
	return 0;
}

uint32_t calcSectorsUsed(size_t size){
	return (size + SECTORSIZE - 1) / SECTORSIZE;
}

void listfiles_fat16(){
	struct File rootSector[16]; //16 files per sector in root directory
	for(uint16_t s = 0; s < 32; s++){
		disk_read_sector(ROOTSECTOR + s, (uint8_t*)rootSector);
		//files should be contiguous so return upon first 0
		for(uint32_t e = 0; e < 16; e++){
			struct File* entry = &rootSector[e];
			print_hex8(entry->filename[0]);
			print(" ");
			if(entry->filename[0] == 0){
				print("No more files");
				return;
			} else{
				print(" ");
				char filename[9];
				char ext[4];
				memcpy(filename, entry->filename, 8);
				memcpy(ext, entry->extension, 3);

				filename[8] = '\0';
				ext[3] = '\0';
				print(filename);
				print(".");
				print(ext);
				print(" ");
			}
		}
	}
}

void listfiles(){
	uint8_t sector[SECTORSIZE];
	disk_read_sector(0, sector);
	// casting sector as array of fileObjects 
	struct FileObject* files = (struct FileObject*)sector;
	print("Files: \n");
	for(int i = 0; i < MAXFILES; i++){
		if(files[i].name[0] != '0'){
			print(files[i].name);
			print("  ");
		}
	}
	return;
}

size_t read_file(const char* filename){
	uint8_t sector[SECTORSIZE];
	disk_read_sector(0, sector);
	struct FileObject* files = (struct FileObject*)sector;

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

				for(int j = 0; j < SECTORSIZE; j++){
					print_hex8(sector[j]);
					print(" ");
				}

				print((char*)sector);

			}
			return 0;
		}
	}
	print("File Not Found\n");
	return 0;
}



//this just finds the next open sector and writes there use overwrite_file to overwrite
// does write more than one sector if needed
void write_file(const char* filename, uint8_t* buffer, size_t size){
	//idea, read sector table, find next open sector write to that
	uint8_t sector[SECTORSIZE];
	disk_read_sector(0, sector);
	struct FileObject* files = (struct FileObject*)sector;
	uint32_t prevSector = 0;
	for(int i = 0; i < MAXFILES; i++){
		if(files[i].name[0] == 0){
			strcpy(files[i].name,filename);
			files[i].startSector = ++prevSector;
			files[i].size = size;

			//updating file table
			disk_write_sector(0, sector);
			//writing to file sector location
			uint32_t secs = calcSectorsUsed(size);
			for(uint32_t j = 0; j < secs; j++){
				disk_write_sector(prevSector + j, buffer + j * SECTORSIZE);
			}
			return;
		}

		//otherwise sector wasnt found
		prevSector += calcSectorsUsed(files[i].size);
	}

	print("Error writing to file");
}


void overwrite_file(const char* filename, uint8_t* buffer, size_t newSize){
	uint8_t sector[SECTORSIZE];
	disk_read_sector(0, sector);
	struct FileObject* files = (struct FileObject*)sector;
	for(int x = 0; x < MAXFILES; x++){
		if(strcmp(filename, files[x].name) == 0){//found file
			files[x].size = newSize;
			uint8_t curSector = files[x].startSector;
			size_t secs = calcSectorsUsed(newSize);
			for(size_t y = 0; y < secs; y++){
				disk_write_sector(curSector, buffer + (y * SECTORSIZE)); // moving through buffer correctly
				curSector++;
			}
			disk_write_sector(0, sector);
			return;
		}
	}
	write_file(filename, buffer, newSize);
}

//read sector table, find startSector, calcSectorsUsed, zero out with memset() by looping, write sector table back
boolean delete_file(const char* filename){
	uint8_t sector[SECTORSIZE];
	disk_read_sector(0, sector);
	struct FileObject* files = (struct FileObject*)sector;

	uint8_t zeroBuffer[SECTORSIZE];
	memset(zeroBuffer, 0, SECTORSIZE);

	for(int i = 0; i < MAXFILES; i++){
		if(strcmp(files[i].name, filename) == 0){
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
	struct FileObject* files = (struct FileObject*)sector;

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
				memcpy(buffer + bytes_copied, curSector, bytes_to_copy);
				bytes_copied += bytes_to_copy;
			}
			return buffer;
		}
	}

	print("File Not Found\n");
	return NULL;
}


