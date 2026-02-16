#pragma once
#include "asm_token.h"
#include "vector.h"


typedef enum {
	INST_INVALID = 0,
	INST_MOV,
	INST_ADD,
	INST_SUB,
	INST_JMP,
	INST_CALL,
	INST_RET,
	INST_PUSH,
	INST_POP,
	INST_CMP,
	INST_JE,
	INST_JNE,
	INST_NOP
} MnemonicType;

typedef struct {
	TokenType type;
	union{
		char* strValue; //register / label name
		int intValue; // immediate value / memory address
	};
	int line;
} Operand;

typedef struct Instruction {
	MnemonicType mnemonic; // mov jmp org lidt lgdt etc
	Operand operand1; // necessary for every instruction
	Operand operand2; //optional
	int operandCount;
	int size;
	int address;
} Instruction;


MnemonicType strToInstructionType(const char* str);
Token advance(Token* tokenArray, int* index);
Token advanceTokenVector(TokVector* vec, int* position);
Token peek(Token* t, int index);
void expect(Token* tokenArray, int* index, TokenType expectedType);
Operand parseOperand(TokVector* vec, int* pos);
Instruction parseInstruction(TokVector* vec);
void parseLine(Token* tokenArray, int* index, InstructionVector* instVec);
void parseTokenArray(Token* tokenArray, int totalToken, InstructionVector* instVec);


