#pragma once
#include "asm_token.h"

struct Instruction;

typedef struct {
	Token* data;
	size_t size;
	size_t capacity;
} TokVector;

typedef struct {
	struct Instruction* data;
	int size;
	int capacity;
} InstructionVector;

void tokenVecInit(TokVector* vec);
void tokenVecPush(TokVector* vec, Token tok);
void tokenVecFree(TokVector* vec);

void instVecInit(InstructionVector* vec);
void instVecPush(InstructionVector* vec, struct Instruction inst);
void instVecFree(InstructionVector* vec);
