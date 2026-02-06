#include "structures.hpp"
#include "utilities.hpp"
#include "string.hpp"
#include "tools.hpp"


void cmd_help() {
    print(help_response);
}

void cmd_clear() {
    init_screen();
    cursor_pos = 0;
    skip_newline = 1;
}

void cmd_reboot() {
    asm("jmp $0xFFFF, $0");
}


void cmd_echo(){
	char* c = &input_buffer[5];
	while(*c){
		print_char(*c);
		c++;
	}
	//print(&input_buffer[12]);
}

void cmd_calc(char* input_buffer){
	calc(input_buffer);
}

void print_buf(char* input_buffer){
	for(int i = 0; i < buffer_pos; i++){
		print_char(input_buffer[i]);
	}
}


void cmd_hexdump(char* input_buffer){
	//usages
	//hexdump -- defaults to 256 bytes
	// hexdump 0x100000 -- 
	token(input_buffer, ' ');
	const char* arg1 = token(NULL, ' ');
	if(strlen(arg1) < 1){ // ie no arg
		hexdump((void*)0x100000, (size_t)256);
	} else if(strlen(arg1) >= 3 && (arg1[0] == '0' && (arg1[1] == 'x' || arg1[1] == 'X'))){ //checking for x ie 0x123
		uintptr_t address = stoh(arg1); //converting address

		const char* arg2 = token(NULL, ' ');
		if(strlen(arg2) < 1){
			hexdump((void*)address, 256);
		}else{
			int size = atoi(arg2);
			hexdump((void*)address, (size_t)size);
		}
	}
	else{
		print("Usage hexdump <address> <size>");
	}
}


extern "C" void parse_command() {
	if (strcmp(input_buffer, "help") == 0) {
		cmd_help();
	} else if (strcmp(input_buffer, "clear") == 0) {
		cmd_clear();
	} else if (strcmp(input_buffer, "reboot") == 0) {
		cmd_reboot();
	} else if (strncmp(input_buffer, "echo", 4) == 0 || strcmp(input_buffer, "echo") == 0) {
		cmd_echo();
	} else if (strncmp(input_buffer, "calc", 4) == 0){
		calc(input_buffer);
	} else if (strcmp(input_buffer, "printbuf") == 0){
		print_buf(input_buffer);
	} else if (strncmp(input_buffer, "hexdump", 7) == 0) {
		cmd_hexdump(input_buffer);
	}
	else {
		print(unknown_response);
	}
}

