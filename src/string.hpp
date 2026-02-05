#ifndef STRING_H
#define STRING_H

#include "structures.hpp"


// string info functions
size_t strlen(const char* str);
extern "C" int strcmp(const char* s1, const char* s2);
int strncmp(const char* s1, const char* s2, size_t n);

//string modifying functions
char* strcat(char* dst, const char* src);

#endif 
