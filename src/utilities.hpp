#ifndef UTILITIES_H
#define UTILITIES_H

#include "structures.hpp"

extern "C" void print(const char *s1);
extern "C" int strcmp(const char* s1, const char* s2);
extern "C" void init_screen();


extern "C" {
    extern int cursor_pos;
    extern int buffer_pos;
    extern unsigned char skip_newline; //1 byte
    extern char input_buffer[80];
    extern char shell_prompt[];
    extern char help_response[];
    extern char unknown_response[];
}




void updateCursorPos(int newPos);
int isDigit(char digit);
int atoi(const char* str);
void itoa(int num, char* buf);
int newLine(int cursor_pos);
extern "C" void clearBuf(void* ptr, size_t size);

char* token(char* str, const char delim);

void print_char(const char c);
void print_num(int num);
void print_hex(unsigned int hex);
void print_address(uintptr_t address);

#endif
