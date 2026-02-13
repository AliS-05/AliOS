#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "token.h"


extern int line;
extern char* source;
extern int curPos;

Token nextToken() {	
	Token tok;
	tok.line = line;	

	while (isspace(source[curPos])) {
		if (source[curPos] == '\n')
			line++;
		curPos++;
	}	

	char c = source[curPos];	
	if (c == '\0') {
		tok.type = TOK_EOF;
		return tok;
	}	
	// Single character tokens
	switch (c) {
		case '(':
			curPos++;
			tok.type = LPAR;
			return tok;
		case ')':
			curPos++;
			tok.type = RPAR;
			return tok;
		case '{':
			curPos++;
			tok.type = LBRACK;
			return tok;
		case '}':
			curPos++;
			tok.type = RBRACK;
			return tok;
		case ';':
			curPos++;
			tok.type = SEMICOLON;
			return tok;
		}	
	// Identifier or keyword
	if (isalpha(c)) {
		char buffer[64];
		int i = 0;	

		while (isalnum(source[curPos])) {
			buffer[i++] = source[curPos++];
		}

		buffer[i] = '\0';	

		if (strcmp(buffer, "int") == 0) {
			tok.type = INT;
			return tok;
		}
		if (strcmp(buffer, "char") == 0) {
			tok.type = CHAR;
			return tok;
		}
		if (strcmp(buffer, "void") == 0) {
			tok.type = VOID;
			return tok;
		}
		if (strcmp(buffer, "return") == 0) {
			tok.type = RETURN;
			return tok;
		}

		tok.type = IDENTIFIER;
		tok.strValue = strdup(buffer);
		return tok;
	}	

	// Number

	if (isdigit(c)) {
		int value = 0;
		while (isdigit(source[curPos])) {
			value = value * 10 + (source[curPos] - '0');
			curPos++;
		}	
		tok.type = NUMBER;
		tok.intValue = value;
		return tok;
	}	

	// Unknown character
	curPos++;
	tok.type = TOK_EOF;
	return tok;
}

