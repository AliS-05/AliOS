#include "asm_token.h"
#include "asm_lexer.h"
#include "asm_parser.h"
#include "symbol_table.h"
#include "codegen.h"
#include "vector.h"
#include "structures.h"
#include "memory.h"

char* source = NULL; 	
int currentTokenIndex = 0;
int curPos = 0;
long filesize = -1;
int line = 1;
int currentAddress = 0x200000;


void init(const char* buffer, uint32_t size){
	source = (char*)buffer;
	filesize = size;
}


void assemble_buffer(const char* buffer, uint32_t size){
	init(buffer, size);

	Token tok;
	Token* tokenArray = (Token*)malloc(sizeof(Token) * filesize * 2);  
	InstructionVector instVec;
	instVecInit(&instVec);

	int totalTokens = 0;
	//start lexing
	do{
		tok = nextToken();
		tokenArray[totalTokens] = tok;
		totalTokens++;

	} while(tok.type != TOK_EOF);
	
	line = 1;
	currentTokenIndex = 0;
	parseTokenArray(tokenArray, &instVec);

	free(tokenArray);
	
	SymbolTable table;
	symbolTableInit(&table);
	
	// loop over instructionVector looking for INST_LABEL's and filling
	// in addresses
	for(int i = 0; i < instVec.size; i++){
		if(instVec.data[i].mnemonic == INST_LABEL){
			symbolTablePush(&table, instVec.data[i].labelName ,instVec.data[i].address);
		}
	}
	
	ByteVector byteVector;
	ByteVectorInit(&byteVector);

	startCodeGen(&instVec, &table, &byteVector);
	
	// write result to OS filesystem
	// shouldnt i just do this manually in mkfs ?
	//write_file("a.bin", byteVector.data, byteVector.size);
}

