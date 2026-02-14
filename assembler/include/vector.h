#pragma once
#include "token.h"

typedef struct{
	Token* data;
	size_t size;
	size_t capacity;
} TokVector;

void tokenVecInit(TokenVector* vec);
void tokenVectorPush(TokenVector* vec, Token tok);
void tokenVectorFree(TokenVector* vec);
