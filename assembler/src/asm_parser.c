#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "asm_token.h"
#include "parser.h"

//when this is called

//should increment curToken and return tokenArray[curToken] 
// i feel like it might not return anything but im not sure
Token advance(){
	currentTokenIndex++;
	return tokenArray[currentTokenIndex];
}

Token advanceTokenVector(TokenVec* vec, int* position){
	return vec[++(*position)];
}

Token peek(){
	return tokenArray[currentTokenIndex+1];
}
//this should either not advance or calls advance inside im not sure
void expect(TokenType expectedType){
	if(tokenArray[currentTokenIndex+1].type != expectedType){
		printf("Error on line %d: Expected: %s Got: %s\n", \
			line, tokenTypeToString(expectedType), tokenTypeToString(tokenArray[currentTokenIndex+1].type);

	}
	return;
}

Operand parseOperand(TokenVec* vec, int* pos){
	Operand op;
	Token t = vec[pos];
	op.type = t.type;
	// think i need something called a tagged union fml
	if(op.type == NUMBER){
		op.intValue = t.intValue;
	} else{
		op.strValue = t.strValue;
	}
	op.line = t.line;
	return op;
}

Instruction parseInstruction(TokenVec* vec){
	// basically only looking for important stuff
	// mnemonics, register, immediates
	int instructionPos = 0;
	Instruction instruction;
	// NOTE need to add error handling but leave that for later
	// this should always be a mnemonic such as mov or jmp
	instruction.mnemonic = vec[instructionPos].strValue;

	//
	instruction.operand1 = parseOperand(&vec, &instructionPos);
	instruction.operand2 = parseOperand(&vec, &instructionPos);
}

void parseLine(Token* tokenArray){
	TokenVec tokVec;
	tokenVecInit(&tokVec);
	
	int lineCounter == 0; //NOTE this could be an error

	Token curTok = advanceTokenVector(&tokVec, &lineCounter);
	do{
		tokenVecPush(&curTok);
		curTok = advanceTokenVector(&tokVec, &lineCounter);
	}while(curTok.type != NEWLINE);
	
	//now vector either contains {NEWLINE}
	// or something like {MOV EAX COMMA 5 SEMICOLON NEWLINE}
	
	//needs to be ANOTHER vector for instructions
	parseInstruction(&tokVec);
	
	tokenVecFree(&tokVec);
}

void parseTokenArray(Token* tokenArray, int totalToken){
	
}


