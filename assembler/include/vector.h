#pragma once
#include "asm_token.h"

typedef struct {
	Token* data;
	size_t size;
	size_t capacity;
} TokVector;

void tokenVecInit(TokVector* vec);
void tokenVecPush(TokVector* vec, Token tok);
void tokenVecFree(TokVector* vec);
