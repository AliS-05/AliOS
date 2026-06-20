#include "structures.h"
#include "utilities.h"
#include "string.h"
#include "memory.h"
#include "asm_token.h"

extern int line;
extern char* source;
extern int curPos;


Token nextToken() {	
	Token tok;
	tok.line = line;	

	while (source[curPos] == ' ' || source[curPos] == '\t' || source[curPos] == '\r')
		curPos++;

	if (source[curPos] == '\n') {
		line++;
		curPos++;
		tok.type = NEWLINE;
		return tok;
	}

	char c = source[curPos];	
	if (c == '\0') {
		tok.type = TOK_EOF;
		return tok;
	}

	// single character tokens ie + - , etc
	switch (c) {
		case ',':
			curPos++;
			tok.type = COMMA;
			return tok;

		case '+':
			curPos++;
			tok.type = PLUS;
		    	return tok;

		case '-':
		    	curPos++;
		    	tok.type = MINUS;
			return tok;

		case '*':
			curPos++;
			tok.type = STAR;
			return tok;
		case '/':
			curPos++;
			tok.type = DIV;
			return tok;

		case ':':
			curPos++;
			tok.type = COLON;
		        return tok;

		case '[': // dereferencing not supported even though i check for it
			curPos++;
			tok.type = LBRACKET;
			return tok;

		case ']':
			curPos++;
			tok.type = RBRACKET;
			return tok;

		case ';': // comment until end of line
			while (source[curPos] && source[curPos] != '\n')
				curPos++;

	    		return nextToken();
	}
	//didnt match any single char tokens so must be a register or label or immediate
	// Identifier or keyword
	if (isLetter(c) || c == '_') {
		char buffer[64];
		int i = 0;	
		//read into buffer and cmp 
		while (isalnum(source[curPos]) || source[curPos] == '_') {
			buffer[i++] = tolower(source[curPos++]); //advances curPos, reading entire word
		}

		buffer[i] = '\0';	
		if (!strcmp(buffer, "rax") ||
			!strcmp(buffer, "rbx") ||
			!strcmp(buffer, "rcx") ||
		        !strcmp(buffer, "rdx") ||
		        !strcmp(buffer, "eax") ||
		        !strcmp(buffer, "ebx") ||
		        !strcmp(buffer, "ecx") ||
		        !strcmp(buffer, "edx") || 
		        !strcmp(buffer, "esp") || 
		        !strcmp(buffer, "ebp") || 
		        !strcmp(buffer, "esi") || 
		        !strcmp(buffer, "edi")) {
		        tok.type = REGISTER;
		        tok.strValue = strdup(buffer);
		        return tok;
		}
		//else identifier
		tok.type = IDENTIFIER;
		tok.strValue = strdup(buffer);
		return tok;
	}

	// Number ie decimal or hex 0x
	if (isDigit(c)) {
	long value = 0;

	if (c == '0' && source[curPos+1] == 'x') {
		curPos += 2;
		while (isDigit(source[curPos]) || (source[curPos] >= 65 && source[curPos] <=70) || (source[curPos] >= 97 && source[curPos] <=102)){ //if is digit or between a-f or A-F
			value = value * 16 + (isDigit(source[curPos]) ? source[curPos] - '0' : tolower(source[curPos]) - 'a' + 10); //clever single liner to convert hex digits shoutout gpt
			curPos++;
		}
		} else {
			while (isDigit(source[curPos])) {
				value = value * 10 + (source[curPos] - '0');
				curPos++;
			}
		}
		tok.type = NUMBER;
		tok.intValue = value;
		return tok;
	}
	tok.type = INVALID;
	curPos++;
	return tok;
}
