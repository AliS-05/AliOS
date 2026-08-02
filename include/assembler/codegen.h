#pragma once
#include <core/structures.h>
#include <assembler/vector.h>
#include <assembler/symbol_table.h>
typedef struct ByteVector{
	uint8_t* data;
	int size;
	int capacity;
} ByteVector;

void ByteVectorInit(ByteVector* vec);
void ByteVectorPush(ByteVector* vec, uint8_t byte);
void ByteVectorFree(ByteVector* vec);
void ByteVectorWrite32(ByteVector* vec, int value);

void startCodeGen(InstructionVector* instVec, SymbolTable* table, ByteVector* byteVector);

