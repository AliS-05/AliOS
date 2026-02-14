#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "asm_token.h"

// TokenVec vector;
// tokenVectorInit(&vector);

void tokenVecInit(TokenVector* vec){
	vec->size = 0;
	vec->capacity = 16;
	vec->data = malloc(sizeof(Token) * vec->capacity);
}

void tokenVecPush(TokenVector* vec, Token* tok){
	if(size > capacity){
		vec->capacity *= 2
		vec->data = realloc(vec->data, sizeof(Token) * vec->capacity);
	}
	vec[vec->size++] = tok;
}


void tokenVecFree(TokenVector* vec){
	free(vec->data);
}
