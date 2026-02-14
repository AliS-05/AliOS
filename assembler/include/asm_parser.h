#pragma once
#include "asm_token.h"
#include "vector.h"

typedef struct {
	TokenType type;
	union{
		char* strValue; //register / label name
		int intValue; // immediate value / memory address
	};
	int line;
} Operand;

typedef struct {
	char* mnemonic; // mov jmp org lidt lgdt etc
	Operand operand1; // necessary for every instruction
	Operand operand2; //optional
} Instruction;


Token advance(Token* tokenArray, int* index);
Token advanceTokenVector(TokVector* vec, int* position);
Token peek(Token* t, int index);
void expect(Token* tokenArray, int* index, TokenType expectedType);
Operand parseOperand(TokVector* vec, int* pos);
Instruction parseInstruction(TokVector* vec);
void parseLine(Token* tokenArray, int* index);
void parseTokenArray(Token* tokenArray, int totalToken);
