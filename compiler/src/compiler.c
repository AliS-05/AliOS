#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "token.h"
#include "lexer.h"
#include "parser.h"

char* source; 	
int currentTokenIndex = 0;
int curPos = 0;
long filesize = -1;
int line = 1;
//isalpha() isdigit() isalnum() isspace()
// read entire file into memory  small sizes == not an issue

int peek(){
	return source[curPos + 1];
}

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



int main(int argc, char** argv) {
	
	if (argc < 2){
		printf("Usage ./compiler <filename>");
		exit(1);
	}

	init(argv[1]);
	Token tok;
	Token* tokenArray = malloc(sizeof(Token) * filesize);
	int totalTokens = 0;
	//start lexing
	do{
		tok = nextToken();
		tokenArray[totalTokens] = tok;
		totalTokens++;

		printf("Token %d: %s", totalTokens, tokenTypeToString(tok.type));
		if(tok.type == IDENTIFIER){
			printf("%s", tok.strValue);
		}

		if(tok.type == NUMBER){
			printf("%d", tok.intValue);
		}

		printf("\n");

	} while(tok.type != TOK_EOF);
	
	line = 1;
	currentTokenIndex = 0;
	parseTokenArray(tokenArray, totalTokens);


	free(tokenArray);
	return 0;
}

