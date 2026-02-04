#ifndef UTILITIES_H
#define UTILITIES_H

#include "structures.hpp"

extern "C" void print(const char *s1);
extern "C" int strcmp(char* s1, const char* s2);
extern "C" void parse_command();


void cmd_help();
void cmd_clear();
void cmd_reboot();


int isDigit(char digit);
int atoi(const char* str);
void itoa(int num, char* buf);

void print_char(const char c);
void print_num(int num);
void print_hex(unsigned int hex);
void print_address(uintptr_t address);

#endif
