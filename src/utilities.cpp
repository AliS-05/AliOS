#include "utilities.hpp"

extern "C" {
    extern unsigned short cursor_pos; //2 bytes
    extern unsigned short buffer_pos; //2 bytes
    extern unsigned char skip_newline; //1 byte
    extern char input_buffer[80];
    extern char shell_prompt[];
    extern char help_response[];
    extern char unknown_response[];
    extern void print_string(const char* str, unsigned short pos);
    extern void init_screen();
}


extern "C" void print_string(const char *s1, unsigned short pos){
	volatile unsigned char* screen = (volatile unsigned char*) (0xB8000 + pos);
	while( *s1 != 0 && pos < 4000){
		*screen = *s1;
		*(screen + 1) = 0x0F;
		s1++;
		screen += 2;
		pos += 2;
	}
	cursor_pos = pos;
}

extern "C" int strcmp(char* s1, const char* s2){
	while (*s1 && *s2 && (*s1 == *s2)){
		s1++;
		s2++;
	}
	return (unsigned char)*s1 - (unsigned char)*s2; // returns non zero value if no match?
}
void cmd_help() {
    print_string(help_response, cursor_pos);
}

void cmd_clear() {
    init_screen();
    cursor_pos = 0;
    skip_newline = 1;
}

void cmd_reboot() {
    asm("jmp $0xFFFF, $0");
}

extern "C" void parse_command() {
    if (strcmp(input_buffer, "help") == 0) {
        cmd_help();
    } else if (strcmp(input_buffer, "clear") == 0) {
        cmd_clear();
    } else if (strcmp(input_buffer, "reboot") == 0) {
        cmd_reboot();
    } else {
        print_string(unknown_response, cursor_pos);
    }
}

int isDigit(char digit){
	if(digit >= '0'&& digit <= '9'){
		return 1;
	}
	return 0;
}

int atoi(const char* str){
	int res = 0;
	while(*str == ' ') str++;
	int sign = 1;
	
	if(*str == '-'){
		sign = -1;
		str++;
	}
	
	while (isDigit(*str)) {
		res = res * 10 + (*str - '0'); // considers digit place and adds ones place
		str++;
	}
	return res * sign;
}

void strtonum(unsigned int num, char* buf){
	if (num == 0){
		buf[0] = '0';
		buf[1] = 0;
		return;
	}

	int i = 0;
	int is_neg = 0;
	if(num < 0){
		is_neg = 1;
		num = -num;
	}

	while(num > 0){
		buf[i++] = (num % 10) + '0'; //increments i
		num /= 10;
	}

	if(is_neg) buf[i++] = '-';
	buf[i] = 0;

	for (int j = 0; j < i/2 ;j++){ // 
		char temp = buf[j];
		buf[j] = buf[i-1-j];
		buf[i-1-j] = temp;
	}
}

void print_num(unsigned int num){
	char buf[20];
	strtonum(num, buf);
	print_string(buf, cursor_pos);
}
