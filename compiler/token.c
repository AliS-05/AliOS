#include "token.h"

const char* tokenTypeToString(TokenType type) {
	switch (type) {
		case IDENTIFIER: return "IDENTIFIER";
		case NUMBER: return "NUMBER";
		case INT: return "INT";
		case CHAR: return "CHAR";
		case VOID: return "VOID";
		case LPAR: return "LPAR";
		case RPAR: return "RPAR";
		case LBRACK: return "LBRACK";
		case RBRACK: return "RBRACK";
		case RETURN: return "RETURN";
		case SEMICOLON: return "SEMICOLON";
		case TOK_EOF: return "TOK_EOF";
		default: return "UNKNOWN";
	}
}

