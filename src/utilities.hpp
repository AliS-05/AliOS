#ifndef UTILITIES_H
#define UTILITIES_H

extern "C" void print(const char *s1, unsigned short pos);
extern "C" int strcmp(char* s1, const char* s2);
extern "C" void parse_command();
void cmd_help();
void cmd_clear();
void cmd_reboot();
int isDigit(char digit);
int atoi(const char* str);
void strtonum(unsigned int num, char* buf);
void print_num(unsigned int num);


#endif
