#include <assembler/asm_token.h>
#include <assembler/asm_lexer.h>
#include <assembler/asm_parser.h>
#include <assembler/symbol_table.h>
#include <assembler/codegen.h>
#include <assembler/vector.h>
#include <core/structures.h>
#include <core/memory.h>
#include <fs/fs.h>
#include <fs/fat16.h>
#include <core/utilities.h>

char* source = NULL; 	
int currentTokenIndex = 0;
int curPos = 0;
int line = 1;
int currentAddress = 0x200000;

//basically the main.c file, calls all other phases of the assembler
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

	} while(tok.type != TOK_EOF && totalTokens < 2048);
	
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
	print("Writing to output file\n");
	writeFile("asoutput", "exe", byteVector.data, byteVector.size);
	print("Finished writing to output file. Enjoy!\n");
}

