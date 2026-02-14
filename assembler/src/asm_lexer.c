#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
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

	// Single character tokens

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

		case ':':
			curPos++;
			tok.type = COLON;
		        return tok;

		case '[':
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

	// Identifier or keyword
	if (isalpha(c) || c == '_') {
		char buffer[64];
		int i = 0;	

		while (isalnum(source[curPos]) || source[curPos] == '_') {
			buffer[i++] = tolower(source[curPos++]);
		}

		buffer[i] = '\0';	

		if (!strcmp(buffer, "rax") ||
		    !strcmp(buffer, "rbx") ||
		    !strcmp(buffer, "rcx") ||
		    !strcmp(buffer, "rdx") ||
		    !strcmp(buffer, "eax") ||
		    !strcmp(buffer, "ebx") ||
		    !strcmp(buffer, "ecx") ||
		    !strcmp(buffer, "edx")) {
		    tok.type = REGISTER;
		    tok.strValue = strdup(buffer);
		    return tok;
		}

		tok.type = IDENTIFIER;
		tok.strValue = strdup(buffer);
		return tok;
	    }

	    // Number (decimal or hex 0x)
	    if (isdigit(c)) {
		long value = 0;

		if (c == '0' && source[curPos+1] == 'x') {
		    curPos += 2;
		    while (isxdigit(source[curPos])) {
			value = value * 16 +
			    (isdigit(source[curPos])
			    ? source[curPos] - '0'
			    : tolower(source[curPos]) - 'a' + 10);
			curPos++;
		    }
		} else {
		    while (isdigit(source[curPos])) {
			value = value * 10 + (source[curPos] - '0');
			curPos++;
		    }
		}

		tok.type = NUMBER;
		tok.intValue = value;
		return tok;
	    }
}
