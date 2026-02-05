#include "structures.hpp"
#include "utilities.hpp"
#include "string.hpp"

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

int tokenize(char* buffer){
	int count = 1;
	char* c = buffer;
	size_t len = strlen(buffer);
	print_num((int)len);
	for(int i = 0; i < 80; i++){
		print_char(*c);
		c++;
	}
	return count;
}

void cmd_echo(){
	char* c = &input_buffer[5];
	while(*c){
		print_char(*c);
		c++;
	}
	//print(&input_buffer[12]);
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
	} else {
		print(unknown_response);
	}
}

