#ifndef STRING_H
#define STRING_H

#include <core/structures.h>


// string info functions
size_t strlen(const char* str);
int strcmp(const char* s1, const char* s2);
int strncmp(const char* s1, const char* s2, size_t n);

//string modifying functions
char* strcat(char* dst, const char* src);
char* strcpy(char* dst, const char* src);

//other
char* strdup(const char* src);
char tolower(char c);
#endif 
