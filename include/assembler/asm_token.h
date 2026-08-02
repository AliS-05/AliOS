#pragma once


typedef enum {
	TOK_EOF,
	NEWLINE,
	IDENTIFIER,
	REGISTER,
	NUMBER,
	COMMA,
	PLUS,
	MINUS,
	STAR,
	DIV,
	COLON,
	LBRACKET, // [
	RBRACKET, // ]
	MEMORY,
	INVALID
} TokenType;

typedef struct {
	TokenType type;
	int line;
	union{
		char* strValue;
		long intValue;
	};
} Token;


const char* tokenTypeToString(TokenType type);
