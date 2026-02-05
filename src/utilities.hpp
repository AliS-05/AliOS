#ifndef UTILITIES_H
#define UTILITIES_H

#include "structures.hpp"

extern "C" void print(const char *s1);
extern "C" int strcmp(const char* s1, const char* s2);


int isDigit(char digit);
int atoi(const char* str);
void itoa(int num, char* buf);
unsigned short newLine(unsigned short cursor_pos);
void clearBuf(void* ptr, size_t size);

char* token(char* str, const char delim);

void print_char(const char c);
void print_num(int num);
void print_hex(unsigned int hex);
void print_address(uintptr_t address);

#endif
