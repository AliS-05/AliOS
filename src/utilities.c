#include <utilities.h>
#include <string.h>
#include <memory.h>
extern int cursor_pos;
extern int buffer_pos;
extern unsigned char skip_newline; //1 byte
extern char input_buffer[80];
extern char shell_prompt[];
extern char help_response[];
extern char unknown_response[];
extern void print(const char* str);
extern uint8_t vga_color;
extern void putChar(char c);

char terminalScrollBuffer[1000][80] = {0};  
int terminalScrollPosition = 0; 
int scrollView = 0;  

void init_editor_screen(uint8_t colorByte){
	vga_color = colorByte;
	int pos = 0;
	unsigned char* vga = (unsigned char*)0xB8000;
	for(int i = 0; i < 2000; i++){
		vga[pos] = ' ';
		vga[pos+1] = colorByte;
		pos += 2;
	}
	cursor_pos = 0;
}

int newLine(int cursor_pos){
	int next_line = (((cursor_pos / 160) + 1) * 160);
	return next_line;
}


void updateCursorPos(int newPos){
	cursor_pos = newPos;
}


void print_char(const char c){
	if(cursor_pos >= 4000) return;
	volatile unsigned char* vga = (volatile unsigned char*)0xB8000;
	vga[cursor_pos] = (unsigned char)c;
	vga[cursor_pos+1] = vga_color;
	cursor_pos += 2;
}

void renderWindow(int top){
	volatile unsigned char* vga = (volatile unsigned char*)0xB8000;
	int p = 0;
	for(int line = 0; line < 25; line++){
		for(int col = 0; col < 80; col++){
			char c = terminalScrollBuffer[top + line][col];
			if(c == '\0' || c == '\n') c = ' ';
			vga[p]   = (unsigned char)c;
			vga[p+1] = vga_color;
			p += 2;
		}
	}
}

void putChar(char c){
	if(scrollView != 0){                    
		scrollView = 0;
		renderWindow(terminalScrollPosition);
	}
	if(c == '\n'){
		if(cursor_pos / 160 >= 24){     
			terminalScrollPosition++;
			renderWindow(terminalScrollPosition);
			cursor_pos = 24 * 160;
		} else {
			cursor_pos = newLine(cursor_pos);
		}
	} else {
		int row = cursor_pos / 160;
		int col = (cursor_pos / 2) % 80;
		terminalScrollBuffer[terminalScrollPosition + row][col] = c;
		print_char(c);
	}
}

void print(const char *s1){
	while(*s1 != '\0'){
		putChar(*s1);
		s1++;
	}
}

void scrollUp(){
	if(terminalScrollPosition - scrollView <= 0) return;
	scrollView++;
	renderWindow(terminalScrollPosition - scrollView);
}

void scrollDown(){
	if(scrollView == 0) return;
	scrollView--;
	renderWindow(terminalScrollPosition - scrollView);
}


void print_num(int num){
	static char buf[32];
	buf[0] = '\0';
	itoa(num, buf);
	print(buf);
}


void print_hex_n(unsigned int value, int nibbles) {
    const char* hex = "0123456789ABCDEF";
    for (int i = nibbles - 1; i >= 0; i--) {
        unsigned int digit = (value >> (i * 4)) & 0xF;
        print_char(hex[digit]);
    }
}

void print_hex8(unsigned char v) {
    print_hex_n(v, 2);
}

void print_hex16(unsigned short v) {
    print_hex_n(v, 4);
}

void print_hex32(unsigned int v) {
    print_hex_n(v, 8);
}

void print_addr(uintptr_t addr) {
    print("0x");
    print_hex32((unsigned int)addr);
}

void print_byte(unsigned char b){
	if(cursor_pos >= 3998) return;
	print_hex8(b);
	print(" ");
}



boolean isDigit(char digit){
	if(digit >= '0'&& digit <= '9'){
		return true;
	}
	return false;
}


boolean isHex(char digit){
	if(isDigit(digit) || (digit >= 'A' && digit <= 'F') || (digit >= 'a' && digit <= 'f')){
		return true;
	}
	return false;
}

boolean isLetter(char c){
	if((c >= 65 && c <=90) || (c >=97 && c <= 122)){
		return true;
	}
	return false;
}

boolean isalnum(char c){
	if(isHex(c) || isLetter(c)){
		return true;
	}
	return false;
}

uintptr_t stoh(const char* str){ //string to hex
	uintptr_t res = 0;
	while(*str == ' ') str++;
	
	str++; //skipping lead zero
	if(*str != 'x' && *str != 'X'){
		return 0; //what do we return for error? cant be -1
	}

	str++;
	
	while(isHex(*str)){
		res = res * 16 + (*str - '0');
		str++;
	}
	return res;
}

int atoi(const char* str){ //converts a string to an integer
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

void itoa(int num, char* buf) { //converts an integer to a string
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

void clearBuf(void*ptr, size_t size){
	unsigned char* p = (unsigned char*)ptr;
	for(size_t i = 0; i < size; i++){
		p[i] = 0;
	}
}


char* token(char* str, const char delim){ //basically strtok 
	static char* saved_pos = NULL;

	if(str != NULL){ //if str == NULL we want to token the same string again
		saved_pos = str;
	}
	
	if(saved_pos == NULL) return NULL;

	char* token_start = saved_pos;

	while(*saved_pos && *saved_pos != delim){
		saved_pos++;
	}

	if(*saved_pos){
		*saved_pos = '\0';
		saved_pos++;
	}

	//savedpos now at first / next ' '
	return token_start;
}



