#pragma once
typedef enum{
	IDENTIFIER,
	NUMBER,
	INT,
	CHAR,
	VOID,
	LPAR,
	RPAR,
	LBRACK,
	RBRACK,
	RETURN,
	SEMICOLON,
	TOK_EOF
}TokenType;

typedef struct{
	TokenType type;
	union{
		int int_value; //for nums
		char* str_value; //for functions and identifiers
	};
	int line; //for debugging potentially might not implement
}Token;

