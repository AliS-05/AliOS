#include <core/structures.h>
#include <core/memory.h>

#include <assembler/vector.h>
#include <assembler/asm_token.h>
#include <assembler/asm_parser.h>
#include <assembler/symbol_table.h>
// TokenVec vector;
// tokenVectorInit(&vector);

void tokenVecInit(TokVector* vec){
	vec->size = 0;
	vec->capacity = 16;
	vec->data = malloc(sizeof(Token) * vec->capacity);
}

void tokenVecPush(TokVector* vec, Token tok){
	if(vec->size >= vec->capacity){
		vec->capacity *= 2;
		vec->data = realloc(vec->data, sizeof(Token) * vec->capacity);
	}
	vec->data[vec->size++] = tok;
}


void tokenVecFree(TokVector* vec){
	free(vec->data);
}


void instVecInit(InstructionVector* vec){
	vec->size = 0;
	vec->capacity = 16;
	vec->data = malloc(sizeof(Instruction) * vec->capacity);
}

void instVecPush(InstructionVector* vec, Instruction inst){
	if(vec->size >= vec->capacity){
		vec->capacity *= 2;
		vec->data = realloc(vec->data, sizeof(Instruction) * vec->capacity);
	}
	vec->data[vec->size++] = inst; //does increase size automatically when you push
}


void instVecFree(InstructionVector* vec){
	free(vec->data);
}

