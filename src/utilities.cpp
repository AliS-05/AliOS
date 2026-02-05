#include "utilities.hpp"
#include "string.hpp"

extern "C" {
    extern unsigned short cursor_pos; //2 bytes
    extern unsigned short buffer_pos; //2 bytes
    extern unsigned char skip_newline; //1 byte
    extern char input_buffer[80];
    extern char shell_prompt[];
    extern char help_response[];
    extern char unknown_response[];
    extern void print(const char* str);
    extern void init_screen();
}


void print_char(const char c){
	if(cursor_pos >= 4000) return;

	volatile unsigned char* vga = (volatile unsigned char*)0xB8000;
	vga[cursor_pos] = (unsigned char)c;
	vga[cursor_pos+1] = 0x0F;
	cursor_pos += 2;
}


unsigned short newLine(unsigned short cursor_pos){
	unsigned short next_line = (((cursor_pos / 160) + 1) * 160);
	return next_line;
}


extern "C" void print(const char *s1){
	while( *s1 != 0 ){
		if(*s1 == '\n'){
			cursor_pos = newLine(cursor_pos);
		}else{
			print_char(*s1);
		}
		s1++;
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

void itoa(int num, char* buf) {
    int i = 0;
    unsigned int n; // Use unsigned to handle INT_MIN safely

    if (num == 0) {
        buf[i++] = '0';
        buf[i] = 0;
        return;
    }

    if (num < 0) {
        buf[i++] = '-';
        n = (unsigned int)(-num);
    } else {
        n = (unsigned int)num;
    }

    // Now work with 'n' (unsigned)
    int start_index = i; // Save where the digits actually start
    while (n > 0) {
        buf[i++] = (n % 10) + '0';
        n /= 10;
    }
    buf[i] = 0;

    // Reverse ONLY the digits, leave the '-' alone at buf[0]
    int end_index = i - 1;
    while (start_index < end_index) {
        char temp = buf[start_index];
        buf[start_index] = buf[end_index];
        buf[end_index] = temp;
        start_index++;
        end_index--;
    }
}

void print_num(int num){
	static char buf[32];
	itoa(num, buf);
	print(buf);
}


void print_hex(unsigned int hex){
	size_t len = strlen("0123456789ABCDEF");
	char chars[len];
	
	print_char('0');
	print_char('x');
	for (int i = 6; i >= 0; i--) {
		// Shift and mask to get each 4-bit nibble
		print_char(chars[(hex >> (i * 4)) & 0xF]);
	}
}

void clearBuf(void*ptr, size_t size){
	unsigned char* p = (unsigned char*)ptr;
	for(size_t i = 0; i < size; i++){
		p[i] = 0;
	}
}
