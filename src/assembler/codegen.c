#include "structures.h"
#include "utilities.h"
#include "memory.h"
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

	print("Unknown register: ");
	print(reg);
	print("\n");
	return -1;
}

uint8_t getMod(int mod, int reg, int rm) {
	return (mod << 6) | (reg << 3) | rm;
}

void encodeMove(Instruction* inst, ByteVector* byteVector){
	// mov reg, [mem]  = 8B
	// mov [mem], reg = 89
	// mov [reg], imm = C7
	if(inst->operand1.type == REGISTER && inst->operand2.type == MEMORY){
		//8B is RM op1 = ModRM:reg, op2 = ModRM:r/m
		int dst = getRegisterCode(inst->operand1.strValue);
		int src = getRegisterCode(inst->operand2.strValue);
		ByteVectorPush(byteVector, 0x8B);
		ByteVectorPush(byteVector, getMod(0b00, dst, src));
		return;
	}
	else if(inst->operand1.type == MEMORY && inst->operand2.type == REGISTER){
		//89 is MR op1 = ModRM:r/m op2 = ModRM:reg
		int dst = getRegisterCode(inst->operand1.strValue);
		int src = getRegisterCode(inst->operand2.strValue);
		ByteVectorPush(byteVector, 0x89);
		//NOTE might need to switch src dst here
		ByteVectorPush(byteVector, getMod(0b00, src, dst));
		return;
	}
	else if(inst->operand1.type == MEMORY && inst->operand2.type == NUMBER){
		//C7 is MI op1 = ModRM:r/m op2 = imm32
		int dst = getRegisterCode(inst->operand1.strValue);
		ByteVectorPush(byteVector, 0xC7);
		ByteVectorPush(byteVector, getMod(0b00, 0, dst));
		ByteVectorWrite32(byteVector, inst->operand2.intValue);
		return;
	}

	int reg1 = getRegisterCode(inst->operand1.strValue);

	if(inst->operand1.type == REGISTER && inst->operand2.type == NUMBER){
		// b8 for immediate to register
		ByteVectorPush(byteVector, 0xB8 + reg1); // b9 ba etc
		ByteVectorWrite32(byteVector, inst->operand2.intValue);
		return;
	}else if(inst->operand1.type == REGISTER && inst->operand2.type == REGISTER){
		int reg2 = getRegisterCode(inst->operand2.strValue);
		ByteVectorPush(byteVector, 0x89);
		//mod rm calculations
		uint8_t modrm = 0xC0 | (reg2 << 3) | reg1; 
		ByteVectorPush(byteVector, modrm);
		return;
	}
}



//http://ref.x86asm.net/coder32.html
//such a goated website
void encodeInstruction(Instruction* inst, SymbolTable* table, ByteVector* byteVector){
	//NOTE commenting this out but might want later
//	print("DEBUG encode Instruction mnemonic value: ");
//	print(inst->mnemonic);
//	print("\n");
	switch(inst->mnemonic) {
		case INST_LABEL: {
			break;
		}
		case INST_MOV: {
			encodeMove(inst, byteVector);
			break;
		}
		case INST_RET: {
			ByteVectorPush(byteVector, 0xC3);
			break;
		}
		case INST_JMP: {
			ByteVectorPush(byteVector, 0xE9);
			//formula for jmp = target - (currentAddress + 5) (opcode + 4 byte address)
			// so if start is at 0 and jmp is at byte 7
			// 0 - (12) == -12 bytes to get to start label (since we have to add jmp's bytes as well)
			// negatives are automatically calculated and stored as twos complement ! so no need to implement functions for that
			int laddr = symbolTableLookup(table, inst->operand1.strValue);
			ByteVectorWrite32(byteVector, (laddr - (inst->address + 5)));
			break;
		}

		case INST_ADD: {
			if(inst->operand1.type == MEMORY){
				//opcode 01 for op2 = r16/32
				if(inst->operand2.type == REGISTER){ 
					//add [eax], ebx
					ByteVectorPush(byteVector, 0x01);

					int sourceReg = getRegisterCode(inst->operand2.strValue);
					int destinationReg = getRegisterCode(inst->operand1.strValue);

					uint8_t modrm = getMod(0, sourceReg, destinationReg);
					ByteVectorPush(byteVector, modrm);
					break;
				} else{ 
					//else immediate ie add [eax], 5
					//0x81
					ByteVectorPush(byteVector, 0x81);
					int destinationReg = getRegisterCode(inst->operand1.strValue);
					uint8_t modrm = getMod(0, 0, destinationReg); // 11101xx
					ByteVectorPush(byteVector, modrm);
					ByteVectorWrite32(byteVector, inst->operand2.intValue);
					break;
				}

			}	
			//add eax, [myVar]
			//add dest, source
			else if(inst->operand1.type == REGISTER && inst->operand2.type == MEMORY){ // REG MEM case 0x03
				ByteVectorPush(byteVector, 0x03);
				int destinationReg = getRegisterCode(inst->operand1.strValue);
				int sourceReg = getRegisterCode(inst->operand2.strValue);
				uint8_t modrm = getMod(0, destinationReg, sourceReg);
				ByteVectorPush(byteVector, modrm);
				break;
			}
			//NOTE check if operand is REG REG or REG IMM. REG REG == 0x01 REG IMM == 0x81
			else if(inst->operand1.type == REGISTER && inst->operand2.type == NUMBER){
				ByteVectorPush(byteVector, 0x81);
				int reg1 = getRegisterCode(inst->operand1.strValue);
				uint8_t modrm = getMod(3, 0, reg1); // 11101xx
				ByteVectorPush(byteVector, modrm);
				ByteVectorWrite32(byteVector, inst->operand2.intValue);
				break;
			} 
			else if(inst->operand1.type == REGISTER && inst->operand2.type == REGISTER){
				ByteVectorPush(byteVector, 0x01);
				int destinationReg = getRegisterCode(inst->operand1.strValue);
				int sourceReg = getRegisterCode(inst->operand2.strValue);
				uint8_t modrm = getMod(3, sourceReg, destinationReg);
				ByteVectorPush(byteVector, modrm);
				break;
			}
			break;
		}

		case INST_SUB: {
			// REG IMM == 0x81 REG REG == 0x29 ?
			if(inst->operand1.type == MEMORY && inst->operand2.type == NUMBER){
				//sub [eax], 8
				ByteVectorPush(byteVector, 0x81);
				int destinationReg = getRegisterCode(inst->operand1.strValue);
				//101 = 5 specifies SUB in op-family 0x81
				uint8_t modrm = getMod(0, 0b101, destinationReg);
				ByteVectorPush(byteVector, modrm);
				ByteVectorWrite32(byteVector, inst->operand2.intValue);
				break;
			}
			if(inst->operand1.type == MEMORY && inst->operand2.type == REGISTER){
				//sub [eax], ebx
				ByteVectorPush(byteVector, 0x29);
				int destinationReg = getRegisterCode(inst->operand1.strValue);
				int sourceReg = getRegisterCode(inst->operand2.strValue);
				uint8_t modrm = getMod(0, destinationReg, sourceReg);
				ByteVectorPush(byteVector, modrm);
				break;
			}
			else if(inst->operand1.type == REGISTER && inst->operand2.type == MEMORY){
				// sub eax, [ebx]
				ByteVectorPush(byteVector, 0x2B);
				int destinationReg = getRegisterCode(inst->operand1.strValue);
				int sourceReg = getRegisterCode(inst->operand2.strValue);
				uint8_t modrm = getMod(0, destinationReg, sourceReg);
				ByteVectorPush(byteVector, modrm);
				break;
			}
			else if(inst->operand1.type == REGISTER && inst->operand2.type == NUMBER){
				ByteVectorPush(byteVector, 0x81);
				int destinationReg = getRegisterCode(inst->operand1.strValue);
				uint8_t modrm =	getMod(3, 0b101, destinationReg);
				ByteVectorPush(byteVector, modrm);
				ByteVectorWrite32(byteVector, inst->operand2.intValue);
				break;
			} else if(inst->operand1.type == REGISTER && inst->operand2.type == REGISTER){
				// sub eax, ebx = eax - ebx
				ByteVectorPush(byteVector, 0x29);
				int destinationReg = getRegisterCode(inst->operand1.strValue);
				int sourceReg = getRegisterCode(inst->operand2.strValue);
				uint8_t modrm = getMod(3, destinationReg, sourceReg);
				ByteVectorPush(byteVector, modrm);
				break;
			}
			break;
		}

		case INST_CMP: {
			if(inst->operand1.type == MEMORY && inst->operand2.type == REGISTER){
				ByteVectorPush(byteVector, 0x39);
				int reg1 = getRegisterCode(inst->operand1.strValue);
				int reg2 = getRegisterCode(inst->operand2.strValue);
				uint8_t modrm = getMod(0, reg2, reg1);
				ByteVectorPush(byteVector, modrm);
				break;
			}
			else if(inst->operand1.type == REGISTER && inst->operand2.type == REGISTER){
				ByteVectorPush(byteVector, 0x39);
				int reg1 = getRegisterCode(inst->operand1.strValue);
				int reg2 = getRegisterCode(inst->operand2.strValue);
				uint8_t modrm = getMod(3, reg2, reg1);
				ByteVectorPush(byteVector, modrm);
				break;
			} else if(inst->operand1.type == REGISTER && inst->operand2.type == MEMORY){
				//CMP eax, [ebx]
				ByteVectorPush(byteVector, 0x3B);
				int reg1 = getRegisterCode(inst->operand1.strValue);
				int reg2 = getRegisterCode(inst->operand2.strValue);
				uint8_t modrm = getMod(0, reg1, reg2);
				ByteVectorPush(byteVector, modrm);
				break;
			} else if(inst->operand1.type == REGISTER && inst->operand2.type == NUMBER){
				ByteVectorPush(byteVector, 0x81);
				int destReg = getRegisterCode(inst->operand1.strValue);
				//op-family 0x81 code 7
				uint8_t modrm = getMod(0b011, 0b111, destReg);
				ByteVectorPush(byteVector, modrm);
				ByteVectorWrite32(byteVector, inst->operand2.intValue);
				break;
			} else if(inst->operand1.type == MEMORY && inst->operand2.type == NUMBER){
				ByteVectorPush(byteVector, 0x81);
				int destReg = getRegisterCode(inst->operand1.strValue);
				//op-family 0x81 code 7
				uint8_t modrm = getMod(0b0, 0b111, destReg);
				ByteVectorPush(byteVector, modrm);
				ByteVectorWrite32(byteVector, inst->operand2.intValue);
				break;
			}
			break;
		}
		case INST_CALL: {
			ByteVectorPush(byteVector, 0xE8);
			int laddr = symbolTableLookup(table, inst->operand1.strValue);
			int rel = laddr - (inst->address + 5);
			ByteVectorWrite32(byteVector, rel);
			break;
		}
		case INST_JE: {
			ByteVectorPush(byteVector, 0x0F);
			ByteVectorPush(byteVector, 0x84);
			int laddr = symbolTableLookup(table, inst->operand1.strValue);
			int rel = laddr - (inst->address + 6);
			ByteVectorWrite32(byteVector, rel);
			break;
		}
		case INST_PUSH: {
			if(inst->operand1.type == REGISTER){
				int reg = getRegisterCode(inst->operand1.strValue);
				ByteVectorPush(byteVector, 0x50 + reg);
			}
			break;
		}
		case INST_POP: {
			if(inst->operand1.type == REGISTER){
				int reg = getRegisterCode(inst->operand1.strValue);
				ByteVectorPush(byteVector, 0x58 + reg);
			}
			break;
		}
		case INST_JNE: {
			ByteVectorPush(byteVector, 0x0F);
			ByteVectorPush(byteVector, 0x85);
			int laddr = symbolTableLookup(table, inst->operand1.strValue);
			int rel = laddr - (inst->address + 6);
			ByteVectorWrite32(byteVector, rel);
			break;
		}
		case INST_NOP: {
			ByteVectorPush(byteVector, 0x90);
			break;
		}
		default: {
			print("Error encoding instruction from codegen.c\n");
			return;
			break;
		}
	}
}


//loop through instruction vector, translate instruction based on mnemonic register / immediate etc, look up labels in symbolTable,  emit code byte, 
void startCodeGen(InstructionVector* instVec, SymbolTable* table, ByteVector* byteVector){
	for(int i = 0; i < instVec->size; i++){
		encodeInstruction(&instVec->data[i], table, byteVector);
	}
}
