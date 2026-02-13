#pragma once
#include "token.h"

void expect(Token* tokenArray, TokenType expectedType, int totalTokens);
void parseTokenArray(Token* tokenArray, int totalTokens);
