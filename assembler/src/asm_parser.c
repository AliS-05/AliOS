#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "asm_token.h"
#include "asm_parser.h"
#include "vector.h"

extern long line;

MnemonicType strToInstructionType(const char* str) {
	if (!strcmp(str, "mov"))  return INST_MOV;
	if (!strcmp(str, "add"))  return INST_ADD;
	if (!strcmp(str, "sub"))  return INST_SUB;
	if (!strcmp(str, "jmp"))  return INST_JMP;
	if (!strcmp(str, "call")) return INST_CALL;
	if (!strcmp(str, "ret"))  return INST_RET;
	if (!strcmp(str, "push")) return INST_PUSH;
	if (!strcmp(str, "pop"))  return INST_POP;
	if (!strcmp(str, "cmp"))  return INST_CMP;
	if (!strcmp(str, "je"))   return INST_JE;
	if (!strcmp(str, "jne"))  return INST_JNE;
	if (!strcmp(str, "nop"))  return INST_NOP;

	return INST_INVALID;
}


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
	Instruction instruction = {0};
	instruction.mnemonic = INST_INVALID;
	// NOTE need to add error handling but leave that for later
	// this should always be a mnemonic such as mov or jmp
	
	if(vec->size == 0){
		return instruction;
	}

	//if(vec->data[instructionPos].type != IDENTIFIER){
	//	printf("Error on line: %d, Expected IDENTIFIER/mnemonic got %s\n", vec->data[instructionPos].line, tokenTypeToString(vec->data[instructionPos].type));
	//	exit(1); 
	//}

	//label case
	if(vec->size == 2 && vec->data[0].type == IDENTIFIER && vec->data[1].type == COLON){
		printf("LABEL: %s\n", vec->data[0].strValue);
		return instruction; // basically a skip
	}
	//directives
	if(vec->data[0].type == IDENTIFIER && ((!strcmp(vec->data[0].strValue, "global")) ||(!strcmp(vec->data[0].strValue, "section")))){
		return instruction; //another skip not an instruction
	}
	
	if(vec->data[0].type != IDENTIFIER){ //error not a label directive or mnemonic
		printf("Error line on %d: Expected mnemonic, got %s\n", vec->data[0].line, tokenTypeToString(vec->data[0].type));
		exit(1);
	}
	instructionPos++;
	instruction.mnemonic = strToInstructionType(vec->data[0].strValue);
	// verifying there are more tokens and getting next operand
	if(instructionPos < vec->size){
		instruction.operand1 = parseOperand(vec, &instructionPos);
		instruction.operandCount = 1;
	}
	
	//skipping commma
	if(instructionPos < vec->size && vec->data[instructionPos].type == COMMA){
		instructionPos++;
	}
	
	// verify second token and get second operand
	if(instructionPos < vec->size){
		instruction.operand2 = parseOperand(vec, &instructionPos);
		instruction.operandCount = 2;
	}

	return instruction;
}

void parseLine(Token* tokenArray, int* index, InstructionVector* instVec){
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
		
		if(inst.mnemonic != INST_INVALID){
			instVecPush(instVec, inst);
			printf("Operand 1: %s", tokenTypeToString(inst.operand1.type));
			printf(" Operand 2: %s\n", tokenTypeToString(inst.operand2.type));
		}


		
	}
	if(tokenArray[*index].type == NEWLINE)
		(*index)++;

	tokenVecFree(&tokVec);
}

void parseTokenArray(Token* tokenArray, int totalToken, InstructionVector* instVec){
	int index = 0;
	while(tokenArray[index].type != TOK_EOF){
		parseLine(tokenArray, &index, instVec);	
	}
}


