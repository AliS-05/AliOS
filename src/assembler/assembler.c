#include "asm_token.h"
#include "asm_lexer.h"
#include "asm_parser.h"
#include "symbol_table.h"
#include "codegen.h"
#include "vector.h"
#include "structures.h"
#include "memory.h"
#include "fs.h"
#include "fat16.h"
#include "utilities.h"

char* source = NULL; 	
int currentTokenIndex = 0;
int curPos = 0;
int line = 1;
int currentAddress = 0x200000;


void assemble_buffer(char* buffer){
	source = buffer;
	Token tok;
	//hardcode 2048 token limit, should be fine for this scope
	Token* tokenArray = (Token*)malloc(sizeof(Token) * 2048);   
	InstructionVector instVec;
	instVecInit(&instVec);

	int totalTokens = 0;
	//start lexing
	print("Starting Lexing\n");
	do{
		tok = nextToken();
		tokenArray[totalTokens] = tok;
		totalTokens++;

	} while(tok.type != TOK_EOF);
	
	line = 1;
	currentTokenIndex = 0;
	print("Starting parsing phase..\n");
	parseTokenArray(tokenArray, &instVec);

	free(tokenArray);
	
	SymbolTable table;
	symbolTableInit(&table);
	
	print("Constructing symbol table\n");
	// loop over instructionVector looking for INST_LABEL's and filling
	// in addresses
	for(int i = 0; i < instVec.size; i++){
		if(instVec.data[i].mnemonic == INST_LABEL){
			symbolTablePush(&table, instVec.data[i].labelName ,instVec.data[i].address);
		}
	}
	
	ByteVector byteVector;
	ByteVectorInit(&byteVector);
	
	print("Constructing executable binary\n");
	startCodeGen(&instVec, &table, &byteVector);
	
	// write result to OS filesystem
	// shouldnt i just do this manually in mkfs ?
	//write_file("a.bin", byteVector.data, byteVector.size);

	print("Writing to output file\n");
	writeFile("asoutput", "exe", byteVector.data, byteVector.size);
	print("Finished writing to output file. Enjoy!\n");
}

