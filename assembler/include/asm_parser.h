#pragma once
#include "asm_token.h"


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


Token advance();
Token peek();
void expect(TokenType expectedType);
Operand parseOperand();
Instruction parseInstruction();
void parseLine(Token* tokenArray);
void parseTokenArray(Token* tokenArray, int totalToken);
