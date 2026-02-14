#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "asm_token.h"
#include "asm_parser.h"
#include "vector.h"

extern long line;

Token advance(Token* tokenArray, int* index){
	(*index)++;
	return tokenArray[*index];

}

Token advanceTokVector(TokVector* vec, int* position){
	return vec->data[++(*position)];
}

Token peek(Token* t, int index){
	return t[index+1];
}
//this should either not advance or calls advance inside im not sure
void expect(Token* tokenArray, int* index, TokenType expectedType){
	if(tokenArray[*index].type != expectedType){
		printf("Error on line %ld: Expected: %s Got: %s\n",line, tokenTypeToString(expectedType), tokenTypeToString(tokenArray[*index+1].type));
		exit(1);
	}
	(*index)++;
}

Operand parseOperand(TokVector* vec, int* pos){
	Operand op;
	Token t = vec->data[*pos];
	op.type = t.type;
	op.line = t.line;
	// think i need something called a tagged union fml
	if(t.type == NUMBER){
		op.intValue = t.intValue;
	} else{
		op.strValue = t.strValue;
	}
	(*pos)++;
	return op;
}

Instruction parseInstruction(TokVector* vec){
	// basically only looking for important stuff
	// mnemonics, register, immediates
	int instructionPos = 0;
	Instruction instruction;
	// NOTE need to add error handling but leave that for later
	// this should always be a mnemonic such as mov or jmp
	if(vec->data[instructionPos].type != IDENTIFIER){
		printf("Error on line: %d, Expected IDENTIFIER/mnemonic got %s\n", vec->data[instructionPos].line, tokenTypeToString(vec->data[instructionPos].type));
		exit(1); 
	}

	instruction.mnemonic = vec->data[instructionPos].strValue;
	instructionPos++;

	instruction.operand1 = parseOperand(vec, &instructionPos);
	
	if(vec->data[instructionPos].type == COMMA){
		instructionPos++;
	}
	//validate expected second operator
	if(instructionPos < vec->size){
		Operand o2 = parseOperand(vec, &instructionPos);
		instruction.operand2 = o2;
	}
	return instruction;
}

void parseLine(Token* tokenArray, int* index){
	TokVector tokVec;
	tokenVecInit(&tokVec);

	while(tokenArray[*index].type != NEWLINE &&
	      tokenArray[*index].type != TOK_EOF){

		tokenVecPush(&tokVec, tokenArray[*index]);
		(*index)++;
	}

	// vector should contain something like {MOV EAX COMMA 5 SEMICOLON NEWLINE}
	// or is empty
	if(tokVec.size > 0){
		Instruction inst = parseInstruction(&tokVec);
		printf("Parsed instruction %s\n", inst.mnemonic);

		printf("Operand 1: %s", tokenTypeToString(inst.operand1.type));
		printf(" Operand 2: %s\n", tokenTypeToString(inst.operand2.type));
	}
	if(tokenArray[*index].type == NEWLINE)
		(*index)++;

	tokenVecFree(&tokVec);
}

void parseTokenArray(Token* tokenArray, int totalToken){
	int index = 0;
	while(tokenArray[index].type != TOK_EOF){
		parseLine(tokenArray, &index);	
	}
}


