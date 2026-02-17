#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "codegen.h"
#include "asm_parser.h"
#include "symbol_table.h"
void ByteVectorInit(ByteVector* vec){
	vec->size = 0;
	vec->capacity = 16;
	vec->data = malloc(sizeof(uint8_t) * vec->capacity);
}

void ByteVectorPush(ByteVector* vec, uint8_t byte){
	if(vec->size >= vec->capacity){
		vec->capacity *= 2;
		vec->data = realloc(vec->data, sizeof(uint8_t) * vec->capacity);
	}
	vec->data[vec->size++] = byte;
}

void ByteVectorFree(ByteVector* vec){
	free(vec->data);
}

void ByteVectorWrite32(ByteVector* vec, int32_t value) {
	ByteVectorPush(vec, value & 0xFF);
	ByteVectorPush(vec, (value >> 8) & 0xFF);
	ByteVectorPush(vec, (value >> 16) & 0xFF);
	ByteVectorPush(vec, (value >> 24) & 0xFF);
}

int getRegisterCode(const char* reg) {
	if(!strcmp(reg, "eax")) return 0;
	if(!strcmp(reg, "ecx")) return 1;
	if(!strcmp(reg, "edx")) return 2;
	if(!strcmp(reg, "ebx")) return 3;
	if(!strcmp(reg, "esp")) return 4;
	if(!strcmp(reg, "ebp")) return 5;
	if(!strcmp(reg, "esi")) return 6;
	if(!strcmp(reg, "edi")) return 7;

	printf("Unknown register %s\n", reg);
	exit(1);
}

void encodeMove(Instruction inst, SymbolTable* table, ByteVector* byteVector){
	int reg1 = getRegisterCode(inst.operand1.strValue);

	if(inst.operand2.type == NUMBER){
		// b8 for immediate to register
		ByteVectorPush(byteVector, 0xB8 + reg1); // b9 ba etc
		ByteVectorWrite32(byteVector, inst.operand2.intValue);
	}else{
		int reg2 = getRegisterCode(inst.operand2.strValue);

		ByteVectorPush(byteVector, 0x89);
		
		uint8_t modrm = 0xC0 | (reg2 << 3) | reg1; // ?
		ByteVectorPush(byteVector, modrm);
	}
}

void encodeInstruction(Instruction inst, SymbolTable* table, ByteVector* byteVector){
	switch(inst.mnemonic) {
		case INST_MOV:
			encodeMove(inst, table, byteVector);
			break;
		case INST_RET:
			ByteVectorPush(byteVector, 0xC3);
			break;
		default:
			printf("Error encoding instruction from codegen.c");
			exit(1);

	}
}

//loop through instruction vector, translate instruction based on mnemonic register / immediate etc, look up labels in symbolTable,  emit code byte, 
void startCodeGen(InstructionVector* instVec, SymbolTable* table, ByteVector* byteVector){
	for(int i = 0; i < instVec->size; i++){
		encodeInstruction(instVec->data[i], table, byteVector);
	}
}
