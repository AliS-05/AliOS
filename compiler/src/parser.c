#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "token.h"
#include "codeGen.h"
extern int currentTokenIndex;
extern int line;

void expect(Token* tokenArray, TokenType expectedType, int totalTokens){
	
	if(currentTokenIndex > totalTokens){
		printf("Error: attempting to read another token when totalToken count has been reached.");
		exit(1);
	}

	TokenType nextTokType = tokenArray[currentTokenIndex].type;
	if(nextTokType != expectedType){
		printf("Error on line %d", line);
		exit(1);
	}
	currentTokenIndex++;
	return;
}

void parseTokenArray(Token* tokenArray, int totalTokens){
	//int main(){
	expect(tokenArray, INT,        totalTokens);  
	expect(tokenArray, IDENTIFIER, totalTokens);
	expect(tokenArray, LPAR,       totalTokens );
	expect(tokenArray, RPAR,       totalTokens);
	expect(tokenArray, LBRACK,     totalTokens);
	//interpret inside of main
	expect(tokenArray, RETURN,     totalTokens);
	int returnValue = tokenArray[currentTokenIndex].intValue;
	expect(tokenArray, NUMBER,     totalTokens);
	expect(tokenArray, SEMICOLON,     totalTokens);
	printf("All tokens expected passed!\n");
	printf("Return Value: %d\n", returnValue);

	codeGen(returnValue);

}

