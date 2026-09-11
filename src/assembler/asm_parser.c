#include <core/structures.h>
#include <core/utilities.h>
#include <core/memory.h>
#include <assembler/asm_token.h>
#include <assembler/asm_parser.h>
#include <assembler/vector.h>

extern long line;
extern long currentAddress;

int instructionSize(Instruction* i){
	if(!i) return 0;
	switch(i->mnemonic){
		case INST_INVALID:
			return 0;
		case INST_MOV:
			//B8 00 00 00 00 (little endian immediate)
			// mov reg, imm
			if(i->operand1.type == REGISTER && i->operand2.type == NUMBER){
				i->size = 5;
				return 5;
			//mov reg, reg
			} else if (i->operand1.type == REGISTER && i->operand2.type == REGISTER){
				i->size = 2;
				return 2;
			} else if(i->operand1.type == MEMORY){
				if(i->operand2.type == NUMBER){
					//C7 case, opcode + modrm + imm
					i->size = 6;
					return 6;
				}
				//REGISTER
				else{
					//8B opcode, mov [reg], reg
					i->size = 2;
					return 2;
				}
			} else if(i->operand2.type == MEMORY){
				//8B opcode, mov ebx, [eax]
				i->size = 2;
				return 2;
			}
			break;
		case INST_LABEL:
			return 0;
			break;
		case INST_DIRECTIVE: 
			if(){

			}
		// C3
		case INST_RET:
			i->size = 1;
			return 1;
		// E9 + 32 bit imm
		case INST_JMP:
			i->size = 5;
			return 5; //near jump for now

		case INST_ADD:
			if(i->operand1.type == MEMORY && i->operand2.type == REGISTER){
				i->size = 2;
				return 2;
			} else if(i->operand1.type == MEMORY && i->operand2.type == NUMBER){
				i->size = 6;
				return 6;
			} else if(i->operand1.type == REGISTER && i->operand2.type == MEMORY){
				i->size = 2;
				return 2;
			}else if(i->operand1.type == REGISTER && i->operand2.type == NUMBER){
				i->size = 6;
				return 6;
			}else if(i->operand1.type == REGISTER && i->operand2.type == REGISTER){
				i->size = 2;
				return 2;
			}
		case INST_SUB:
			if(i->operand1.type == MEMORY && i->operand2.type == NUMBER){
				i->size = 6;
				return 6;
			} else if(i->operand1.type == MEMORY && i->operand2.type == REGISTER){
				i->size = 2;
				return 2;
			} else if(i->operand1.type == REGISTER && i->operand2.type == MEMORY){
				i->size = 2;
				return 2;
			}else if(i->operand1.type == REGISTER && i->operand2.type == NUMBER){
				i->size = 6;
				return 6;
			}else if(i->operand1.type == REGISTER && i->operand2.type == REGISTER){
				i->size = 2;
				return 2;
			}
		case INST_CMP:
			if(i->operand1.type == MEMORY && i->operand2.type == REGISTER){
				i->size = 2;
				return 2;
			} else if(i->operand1.type == REGISTER && i->operand2.type == REGISTER){
				i->size = 2;
				return 2;
			} else if(i->operand1.type == REGISTER && i->operand2.type == MEMORY){
				i->size = 2;
				return 2;
			}else if(i->operand1.type == REGISTER && i->operand2.type == NUMBER){
				i->size = 6;
				return 6;
			}else if(i->operand1.type == MEMORY && i->operand2.type == NUMBER){
				i->size = 6;
				return 6;
			}
			break;
		case INST_CALL:
			i->size = 5;
			return 5;
		case INST_JE:
			i->size = 6;
			return 6;
		case INST_PUSH:
		case INST_POP:
			i->size = 1;
			return 1;
		case INST_JNE:
			i->size = 6;
			return 6;
		case INST_INT8:
			//CD imm8
			//one byte for instruction one byte for ISR number
			i->size = 2;
			return 2;
		case INST_INT3:
			//CC 
			//only one byte for instruction
			i->size = 1;
			return 1;
		default:
		case INST_NOP:
			i->size = 1;
			return 1;

			print("Error calculating iruction size\n");
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
	if (!strcmp(str, "int8"))  return INST_INT8;
	if (!strcmp(str, "int3"))  return INST_INT3;

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
		case INT_INT8:  return "int8";
		case INT_INT3:  return "int3";
		default:        return "invalid";
	    }
}


void printInstruction(Instruction* i){
	if(!i) return;
	char buf[32];
	print("Instruction{ ");
	print(mnemonicTypeToStr(i->mnemonic));
	print(" }\n");
	if(i->operandCount >= 1){
		if(i->operand1.type == NUMBER){
			print(" Operand 1 { ");
			print(ntos(i->operand1.intValue, buf, 10));
			print(buf);
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
			print(ntos(i->operand2.intValue, buf, 10));
			
			print(" }\n");
		} else{
			print(" Operand 2 { ");
			print(i->operand2.strValue);
			print(" }\n");
		}
	}
	print("Size of Instruction: ");
	print(ntos(i->size, buf , 10));
	print("\n");
	print("Address of Instruction: ");
	print(ntos(i->address, buf , 10));
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
		char buf[32];
		print(ntos(line, buf, 10));
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

void directive_handler(TokVector* vec){
	//handle myVar db 0 == {IDENTIFIER DIRECTIVE NUMBER NEWLINE}
	//we can do a hacky thing and use the Instruction struct but change how we interpret the fields in the struct
	//i need to do this because we have to create a Instruction struct *now* to add to the INstruction Vector,
	//which we then use to create the symbol table. i want to use the symbol table to store offsets for both labels
	//AND variables.
	// so for example the instruction has our variable name, operand one can be the string value of the directive
	// operand 2 can be the initial value desired. 
	//the important part is that the *size* of the instruction will represent

	//ok scratch all that lets just pass the symbol table directly and emit directive instructions that will emit one explicit byte during codegen. 
	//much more likely to be the correct solution and less error prone. 
	//but how do i return multiple instruction structs... ?
	if(vec->size >= 4 && !strcmp(vec->data[1].strValue, "db")){
		instruction.mnemonic = INST_DIRECTIVE;
		instruction.labelName = vec->data[0].strValue;
		instruction.address
		Operand op1;
		Operand op2;
		op1.type = DIRECTIVE;
		op1.strValue = strdup(vec->data[1].strValue);
		
		op2.type = NUMBER;
		op2.intType = vec->data[2].intValue;
		
		size_t i = 3;
		while(i < vec->size){
			
		}
	}
}

Instruction parseInstruction(TokVector* vec, SymbolTable* symbolTable){
	char buf[32];
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

	//LABELS AND DIRECTIVES, DB, DQ FAMILY

	//if a line starts with a label, skip it, we add to symbol table in next pass
	//NOTE should this be 3 ? vec would be {LABEL COLON NEWLINE} no? i mean it works so i wont touch it just leaving a note
	if(vec->size == 2 && vec->data[0].type == IDENTIFIER && vec->data[1].type == COLON){
		instruction.mnemonic = INST_LABEL;
		instruction.labelName = vec->data[0].strValue;
		print("LABEL: ");
		print(vec->data[0].strValue); 
		print("Address: ");
		print(ntos(currentAddress, buf, 10));
		return instruction;
	}
	//Directives
	// for db it should be defined like label db 0
	//creates a DIRECTIVE instruction that codegen will use to emit single bytes along with symbol table filling in
	if(vec->size == 4 && vec->data[0].type == IDENTIFIER && vec->data[1].type == INST_DIRECTIVE){
		instruction.mnemonic = INST_DIRECTIVE;
		instruction.labelName = vec->data[0].strValue;
		Operand op;
		op.type = DIRECTIVE;
		op.intValue = vec->data[2].intValue;

		instruction.operand1 = op;

		return instruction;
	}

	//above is the basic variable case, more complex variable declarations should go here in the future
	

	if(vec->data[0].type != IDENTIFIER){ //error not a label directive or mnemonic
		print("Error on line: ");
		print(ntos(vec->data[0].line, buf, 10));
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

void parseLine(Token* tokenArray, int* index, InstructionVector* instVec, SymbolTable* symbolTable){
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
		Instruction inst = parseInstruction(&tokVec, &symbolTable);
		if(inst.mnemonic != INST_INVALID){
			inst.address = currentAddress;
			currentAddress += instructionSize(&inst);
			instVecPush(instVec, inst);
			printInstruction(&inst);
		}
	}
	if(tokenArray[*index].type == NEWLINE)
		(*index)++;

	tokenVecFree(&tokVec);
}

void parseTokenArray(Token* tokenArray, InstructionVector* instVec, SymbolTable* symbolTable){
	int index = 0;
	while(tokenArray[index].type != TOK_EOF){
		parseLine(tokenArray, &index, instVec, &symbolTable);	
	}

}
