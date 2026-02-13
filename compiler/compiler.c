#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "token.h"


char* source; 	
int curPos = 0;
long filesize = -1;
int line = 1;
//isalpha() isdigit() isalnum() isspace()
// read entire file into memory  small sizes == not an issue

void init(const char* filename){
	FILE* file = fopen(filename, "rb");
		if(file){
			if(fseek(file, 0, SEEK_END) == 0){
				filesize = ftell(file);
				fseek(file, 0, SEEK_SET);
			}

			source = (char*)malloc(filesize + 1);
			size_t bytes_read = fread(source, sizeof(char), filesize, file);
			source[bytes_read] = '\0';

			fclose(file);
		}
}

Token nextToken() {	
	Token tok;
	tok.line = line;	
	while (isspace(source[curPos])) {
	if (source[curPos] == '\n')
        	line++;
		curPos++;
	}	
	char c = source[curPos];	
	if (c == '\0') {
		tok.type = TOK_EOF;
		return tok;
	}	
	// Single character tokens
	switch (c) {
	case '(':
        	curPos++;
        	tok.type = LPAR;
        	return tok;
	case ')':
        	curPos++;
        	tok.type = RPAR;
        	return tok;
	case '{':
        	curPos++;
        	tok.type = LBRACK;
        	return tok;
	case '}':
        	curPos++;
        	tok.type = RBRACK;
        	return tok;
	case ';':
        	curPos++;
        	tok.type = SEMICOLON;
        	return tok;
	}	
	// Identifier or keyword
	if (isalpha(c)) {
		char buffer[64];
		int i = 0;	
		while (isalnum(source[curPos])) {
			buffer[i++] = source[curPos++];
		}	
		buffer[i] = '\0';	
		if (strcmp(buffer, "int") == 0) {
			tok.type = INT;
			return tok;
		}
		if (strcmp(buffer, "char") == 0) {
			tok.type = CHAR;
			return tok;
		}
		if (strcmp(buffer, "void") == 0) {
			tok.type = VOID;
			return tok;
		}
		if (strcmp(buffer, "return") == 0) {
			tok.type = RETURN;
			return tok;
		}	
		tok.type = IDENTIFIER;
		tok.strValue = strdup(buffer);
		return tok;
	}	
	// Number
	if (isdigit(c)) {
		int value = 0;
		while (isdigit(source[curPos])) {
			value = value * 10 + (source[curPos] - '0');
			curPos++;
		}	
		tok.type = NUMBER;
		tok.intValue = value;
		return tok;
	}	
	// Unknown character
	curPos++;
	tok.type = TOK_EOF;
	return tok;
}

int main(int argc, char** argv) {
	
	init(argv[1]);
	Token tok;

	//start parsing
	do{
		tok = nextToken();
		printf("Token: %s", tokenTypeToString(tok.type));
		if(tok.type == IDENTIFIER){
			printf("%s", tok.strValue);
		}

		if(tok.type == NUMBER){
			printf("%d", tok.intValue);
		}

		printf("\n");

	} while(tok.type != TOK_EOF);
	return 0;
}

