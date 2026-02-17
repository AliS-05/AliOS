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

void ByteVectorWrite32(ByteVector* vec, int value) {
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

void encodeMove(Instruction* inst, SymbolTable* table, ByteVector* byteVector){
	int reg1 = getRegisterCode(inst->operand1.strValue);

	if(inst->operand2.type == NUMBER){
		// b8 for immediate to register
		ByteVectorPush(byteVector, 0xB8 + reg1); // b9 ba etc
		ByteVectorWrite32(byteVector, inst->operand2.intValue);
	}else{
		int reg2 = getRegisterCode(inst->operand2.strValue);

		ByteVectorPush(byteVector, 0x89);
		
		uint8_t modrm = 0xC0 | (reg2 << 3) | reg1; // ?
		ByteVectorPush(byteVector, modrm);
	}
}

void encodeInstruction(Instruction* inst, SymbolTable* table, ByteVector* byteVector){
	printf("DEBUG encodeInstruction mnemonic value: %d\n", inst->mnemonic);
	switch(inst->mnemonic) {
		case INST_LABEL:
			return;
		case INST_MOV:
			encodeMove(inst, table, byteVector);
			break;
		case INST_RET:
			ByteVectorPush(byteVector, 0xC3);
			return;
		case INST_JMP:
			ByteVectorPush(byteVector, 0xE9);
	
			//formula for jmp = target - (currentAddress + 5)
			// so if start is at 0 and jmp is at byte 7
			// 0 - (12) == -12 bytes to get to start label (since we have to add jmp's bytes as well)
			// negatives are automatically calculated and stored as twos complement ! so no need to implement functions for that
			int laddr = symbolTableLookup(table, inst->operand1.strValue);
			ByteVectorWrite32(byteVector, (laddr - (inst->address + 5)));
			break;

		case INST_ADD:
			//NOTE check if operand is REG REG or REG IMM. REG REG == 0x01 REG IMM == 0x81
			ByteVectorPush(byteVector, 0x81);
			int reg1 = getRegisterCode(inst->operand1.strValue);
			uint8_t modrm = 0xC0 | (0 << 3) | reg1;
			ByteVectorPush(byteVector, modrm);
			ByteVectorWrite32(byteVector, inst->operand2.intValue);
			break;
		case INST_SUB:
			ByteVectorPush(byteVector, 0x16);
		case INST_CMP:
			ByteVectorPush(byteVector, 0x38);
		case INST_CALL:
			ByteVectorPush(byteVector, 0x9A);
		case INST_JE:
			ByteVectorPush(byteVector, 0x84);
		default:
			printf("Error encoding instruction from codegen.c\n");
			exit(1);

	}
}

//loop through instruction vector, translate instruction based on mnemonic register / immediate etc, look up labels in symbolTable,  emit code byte, 
void startCodeGen(InstructionVector* instVec, SymbolTable* table, ByteVector* byteVector){
	for(int i = 0; i < instVec->size; i++){
		encodeInstruction(&instVec->data[i], table, byteVector);
	}
}
