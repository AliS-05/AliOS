
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
	unsigned char* screen = (unsigned char*) (0xB8000 + pos);
	int count = 0;
	while( *s1 != 0 ){
		*screen = *s1;
		*(screen + 1) = 0x0F;
		s1++;
		screen += 2;
		count += 2;
	}
	cursor_pos = pos + count;
}

extern "C" int strcmp(char* s1, const char* s2){
	while (*s1 && (*s1 == *s2)){
		s1++;
		s2++;
	}
	return *s1 - *s2; // returns non zero value if no match?
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

extern "C" void kernel_main(){
	print_string("BOOT OK", 0);
	print_string(shell_prompt, 160);
	//unsigned int addr = (unsigned int)shell_prompt;
	//print_string("Address is: ", 0);
	//// We can't print hex yet, so just test if shell_prompt is accessible

	//// Test 2: Access first few characters directly
	//char test[20];
	//test[0] = shell_prompt[0];
	//test[1] = shell_prompt[1];  
	//test[2] = shell_prompt[2];
	//test[3] = 0;
	//print_string(test, 0);

	//// Test 3: Try the original

}
