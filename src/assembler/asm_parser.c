#include "structures.h"
#include "utilities.h"
#include "memory.h"
#include "asm_token.h"
#include "asm_parser.h"
#include "vector.h"

extern long line;
extern long currentAddress;

int instructionSize(Instruction* i, boolean modrmNeeded){
	if(!i) return 0;

	int extra = modrmNeeded ? 1 : 0;

	switch(i->mnemonic){
		case INST_INVALID:
			return 0;
		case INST_MOV:
			//B8 00 00 00 00 (little endian immediate)
			// mov reg, imm
			if(i->operand1.type == REGISTER && i->operand2.type == NUMBER){
				i->size = 5 + extra;
				return 5 + extra;
			//89 /r 
			//mov reg, reg
			} else if (i->operand1.type == REGISTER && i->operand2.type == REGISTER){
				i->size = 2 + extra;
				return 2 + extra;
			} else if(i->operand1.type == MEMORY){
				if(i->operand2.type == NUMBER){
					//C7 case
					i->size = 5 + extra;
					return 5 + extra;
				}
				else{
					//8B opcode, mov [reg], reg
					//displacement byte ?
					i->size = 2 + extra;
					return 2 + extra;
				}
			} else if(i->operand2.type == MEMORY){
				//8B opcode, mov reg, [reg]
				i->size = 2 + extra;
				return 2 + extra;
			}
			break;
		case INST_LABEL:
			return 0;
			break;
		// C3
		case INST_RET:
			i->size = 1 + extra;
			return 1 + extra;
		// E9 + 32 bit imm
		case INST_JMP:
			i->size = 5 + extra;
			return 5 + extra; //near jump for now

		case INST_ADD:
		case INST_SUB:
		case INST_CMP:
			i->size = 2 + extra;
			return 2 + extra;

		case INST_PUSH:
		case INST_POP:
			i->size = 1 + extra;
			return 1 + extra;
		case INST_NOP:
			i->size = 1 + extra;
			return 1 + extra;
		case INST_CALL:
			i->size = 5 + extra;
			return 5 + extra;
		case INST_JE:
		case INST_JNE:
			i->size = 6 + extra;
			return 6 + extra;
		default:
			print("Error calculating instruction size\n");
			return -1;		
	}
	return -1;
}


MnemonicType strToInstructionType(const char* str) {
	if (!strcmp(str, "mov"))  return INST_MOV;
	if (!strcmp(str, "add"))  return INST_ADD;
	if (!strcmp(str, "sub"))  return INST_SUB;
	if (!strcmp(str, "jmp"))  return INST_JMP;
	if (!strcmp(str, "call")) return INST_CALL;
	if (!strcmp(str, "ret"))  return INST_RET;
	if (!strcmp(str, "push")) return INST_PUSH;
	if (!strcmp(str, "pop"))  return INST_POP;
	if (!strcmp(str, "cmp"))  return INST_CMP;
	if (!strcmp(str, "je"))   return INST_JE;
	if (!strcmp(str, "jne"))  return INST_JNE;
	if (!strcmp(str, "nop"))  return INST_NOP;

	return INST_INVALID;
}


const char* mnemonicTypeToStr(MnemonicType type){
	switch(type){
		case INST_LABEL: return "label";
		case INST_MOV:  return "mov";
		case INST_ADD:  return "add";
		case INST_SUB:  return "sub";
		case INST_JMP:  return "jmp";
		case INST_CALL: return "call";
		case INST_RET:  return "ret";
		case INST_PUSH: return "push";
		case INST_POP:  return "pop";
		case INST_CMP:  return "cmp";
		case INST_JE:   return "je";
		case INST_JNE:  return "jne";
		case INST_NOP:  return "nop";
		default:        return "invalid";
	    }
}


void printInstruction(Instruction* i){
	if(!i) return;

	print("Instruction{ ");
	print(mnemonicTypeToStr(i->mnemonic));
	print(" }\n");
	if(i->operandCount >= 1){
		if(i->operand1.type == NUMBER){
			print(" Operand 1 { ");
			print_num(i->operand1.intValue);
			print(" }\n");
		} else{
			print(" Operand 1 { ");
			print(i->operand1.strValue);
			print(" }\n");
		}
	}

	if(i->operandCount >= 2){
		if(i->operand2.type == NUMBER){
			print(" Operand 2 { ");
			print_num(i->operand2.intValue);
			print(" }\n");
		} else{
			print(" Operand 2 { ");
			print(i->operand2.strValue);
			print(" }\n");
		}
	}
	print("Size of Instruction: ");
	print_num(i->size);
	print("\n");
	print("Address of Instruction: ");
	print_num(i->address);
	print("\n");
}

Token advance(Token* tokenArray, int* index){
	(*index)++;
	return tokenArray[*index];

}

Token advanceTokVector(TokVector* vec, int* position){
	return vec->data[++(*position)];
}

Token peek(Token* t, int index){
	return t[index+1];
}

//simple check, if the two types dont match print an error. I dont have exit(1) implemented unfortunately so i think errors will just not really matter
void expect(Token* tokenArray, int* index, TokenType expectedType){
	if(tokenArray[*index].type != expectedType){
		print("Error on line: ");
		print_num(line);
		print("Expected: ");
		print((tokenTypeToString(expectedType)));
		print("Got: ");	
		print(tokenTypeToString(tokenArray[*index].type));
		print("\n");
		return;
	}
	(*index)++;
}

Operand parseOperand(TokVector* vec, int* pos){
	Operand op;
	Token t = vec->data[*pos]; //t is current Token
	op.type = t.type;
	op.line = t.line;
	
	if(t.type == LBRACKET){
		(*pos)++; //skip [
		op.type = MEMORY; // dont want it to stay LBRACKET
		op.strValue = vec->data[*pos].strValue; //copying register value
		(*pos)++; //done with register now sitting at ] which gets skipped below
	}
	else if(t.type == NUMBER){
		op.intValue = t.intValue;
	} else{
		op.strValue = t.strValue;
	}
	(*pos)++;
	return op;
}

Instruction parseInstruction(TokVector* vec){
	// basically only looking for important stuff
	// mnemonics, register, immediates
	int instructionPos = 0;
	Instruction instruction = {0};
	instruction.mnemonic = INST_INVALID;
	instruction.operandCount = 0;
	// NOTE need to add error handling but leave that for later
	// this should always be a mnemonic such as mov or jmp
	
	if(vec->size == 0){
		return instruction;
	}
	//i have no idea what this is rereading it
	//sayinhg if there are 2 tokens and the first oh its skipping start: i think
	if(vec->size == 2 && vec->data[0].type == IDENTIFIER && vec->data[1].type == COLON){
		instruction.mnemonic = INST_LABEL;
		instruction.labelName = vec->data[0].strValue;
		print("LABEL: ");
		print(vec->data[0].strValue); 
		print("Address: ");
		print_num(currentAddress);

		return instruction; // skip adding to instruction vector
	}
	//directives
	if(vec->data[0].type == IDENTIFIER && ((!strcmp(vec->data[0].strValue, "global")) ||(!strcmp(vec->data[0].strValue, "section")))){
		return instruction; //another skip not an instruction
	}
	
	if(vec->data[0].type != IDENTIFIER){ //error not a label directive or mnemonic
		print("Error on line: ");
		print_num(vec->data[0].line);
		print("Expected mnemonic, got: ");
		print(tokenTypeToString(vec->data[0].type));
		return instruction;
	}
	instructionPos++;
	instruction.mnemonic = strToInstructionType(vec->data[0].strValue);
	// verifying there are more tokens and getting next operand
	if(instructionPos < vec->size){
		instruction.operand1 = parseOperand(vec, &instructionPos);
		instruction.operandCount = 1;
	}
	
	//skipping commma 
	if(instructionPos < vec->size && vec->data[instructionPos].type == COMMA){
		instructionPos++;
	}
	
	// verify second token and get second operand
	if(instructionPos < vec->size){
		instruction.operand2 = parseOperand(vec, &instructionPos);
		instruction.operandCount = 2;
	}

	return instruction;
}

void parseLine(Token* tokenArray, int* index, InstructionVector* instVec){
	TokVector tokVec;
	tokenVecInit(&tokVec);
	boolean modrmNeeded = false;

	while(tokenArray[*index].type != NEWLINE &&
	      tokenArray[*index].type != TOK_EOF){

		tokenVecPush(&tokVec, tokenArray[*index]);
		(*index)++;
	}
	// vector should contain something like {MOV EAX COMMA 5 SEMICOLON NEWLINE}
	// or is empty
	
	
	

	if(tokVec.size > 0) {
		Instruction inst = parseInstruction(&tokVec);
		boolean modrmNeeded = (inst.operand1.type == MEMORY || inst.operand2.type == MEMORY); //modrm needed if either operand is a MEMORY type
		if(inst.mnemonic != INST_INVALID){
			inst.address = currentAddress;
			currentAddress += instructionSize(&inst, modrmNeeded);
			instVecPush(instVec, inst);
			printInstruction(&inst);
		}
	}
	if(tokenArray[*index].type == NEWLINE)
		(*index)++;

	tokenVecFree(&tokVec);
}

void parseTokenArray(Token* tokenArray, InstructionVector* instVec){
	int index = 0;
	while(tokenArray[index].type != TOK_EOF){
		parseLine(tokenArray, &index, instVec);	
	}

}


