#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "token.h"


char* source; 	

//isalpha() isdigit() isalnum() isspace()
// read entire file into memory  small sizes == not an issue

char* readIdentifer(int curPos){
	// should read until whitespace and return string
	// int x = 4;
	// ^curpos
	//readIdentifer(curPos) returns 'int'
	

	while(source[curPos] == ' '){ //skipping any initial whitespace
		curPos++;
	}

	char* identifier = (char*)malloc(64);
	int iter = 0;

	while(source[curPos] != ' ' && source[curPos] != '\0'){
		identifier[iter] = source[curPos];
		iter++;
		curPos++;
	}

	identifier[iter] = '\0';

	printf("Identifier read: %s", identifier);
	return identifier;
}

int readNum(int curPos){
	//same as readIdentifer but for numerals
}

Token tokenize(){

}

int main() {
	static int line = 1;
	long filesize = -1;
	int curPos = 0;

	FILE* file = fopen("goal1.c", "rb");
	if(file){
		if(fseek(file, 0, SEEK_END) == 0){
			filesize = ftell(file);
			fseek(file, 0, SEEK_SET);
		}

		source = (char*)malloc(filesize + 1);
		size_t bytes_read = fread(source, sizeof(char), filesize, file);
		source[bytes_read] = '\0';
		rewind(file);

		fclose(file);
	}
	
	//start parsing
	
	readIdentifer(0);

	return 0;
}

