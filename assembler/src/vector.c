#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "asm_token.h"
#include "vector.h"

// TokenVec vector;
// tokenVectorInit(&vector);

void tokenVecInit(TokVector* vec){
	vec->size = 0;
	vec->capacity = 16;
	vec->data = malloc(sizeof(Token) * vec->capacity);
}

void tokenVecPush(TokVector* vec, Token tok){
	if(vec->size > vec->capacity){
		vec->capacity *= 2;
		vec->data = realloc(vec->data, sizeof(Token) * vec->capacity);
	}
	vec->data[vec->size++] = tok;
}


void tokenVecFree(TokVector* vec){
	free(vec->data);
}
