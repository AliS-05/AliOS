#ifndef VECTOR_H
#define VECTOR_H

#include "asm_token.h"
#include "structures.h"
struct Instruction;
struct Symbol;

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

#endif
