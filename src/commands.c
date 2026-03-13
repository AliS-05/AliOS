#include <structures.h>
#include <utilities.h>
#include <string.h>
#include <tools.h>
#include <fs.h>
#include <edit.h>

extern volatile uint8_t enter_editor_flag;

void cmd_help() {
    print(help_response);
    print(" ls hexdump read write del edit");
}

void cmd_clear() {
    init_screen();
    cursor_pos = 0;
    skip_newline = 1;
}

void cmd_reboot() {
    __asm__ __volatile__ ("jmp $0xFFFF, $0");
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

void cmd_ls(){
	listfiles();
}

void cmd_makefile(char* input_buffer){
	size_t inputSize = strlen(input_buffer);
	const char* buf1 = token(input_buffer, ' ');

	const char* filename = token(NULL, ' ');
	if(filename == NULL){
		print("Error finding file to delete");
		return;
	}
	
	uint8_t buffer[SECTORSIZE];
	buffer[0] = '\0';

	size_t offset = strlen(buf1) + strlen(filename) + 2; //i think +2 because of spaces ?
	
	strcpy((char*)buffer, &input_buffer[offset]); //this is how we will write the remaining buffer to disk
					      
	size_t size = inputSize - offset;
	write_file(filename, buffer, size);
}

void cmd_delfile(char* input_buffer){
	token(input_buffer, ' ');
	const char* filename = token(NULL, ' ');
	if(filename == NULL){
		print("Error finding file to delete");
		return;
	}
	delete_file(filename);
}

void cmd_readfile(char* input_buffer){
	token(input_buffer, ' ');
	const char* filename = token(NULL, ' ');
	if(filename == NULL){
		print("Error finding file to read");
		return;
	}
	read_file(filename);
}


void cmd_run(char* input_buffer){

	token(input_buffer, ' ');
	const char* filename = token(NULL, ' ');
	//print("Attempting to run ");
	//print(filename);

	if(!filename){
		print("No file\n");
		return;
	}

	uint8_t* memory = (uint8_t*)0x200000;


	if(!cpy_file_buffer(filename, memory, fileSize(filename))){
		print("Load failed\n");
		return;
	}
	
	//print_num((uint32_t)memory);

	typedef int (*Program)();
	Program program = (Program)memory;

	int ret = program();

	print("Program Finished: ");
	print_num(ret);
}

void cmd_edit(char* input_buffer){
	token(input_buffer, ' ');
	const char* filename =  token(NULL, ' ');
	if(!filename){
		print("Usage edit <filename>");
	}
//	edit_loop(filename);
	extern char editor_filename[32];
	strcpy(editor_filename, filename);
	enter_editor_flag = 1;
}

void cmd_color(char* input_buffer){
	token(input_buffer, ' '); // 'color'
	const char* hexCol = token(NULL, ' '); // something like 0xF1
	int col = atoi(hexCol);
	if(col > 255){
		print("Must enter an integer number, try 'color 02' or 'color 30'");
	}
	init_editor_screen((uint8_t)col);
}

void parse_command() {
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
	} else if (strncmp(input_buffer, "ls", 2) == 0) {
		cmd_ls();
	} else if(strncmp(input_buffer, "del", 3) == 0){
		cmd_delfile(input_buffer);
	} else if (strncmp(input_buffer, "write", 5) == 0){
		cmd_makefile(input_buffer);
	} else if (strncmp(input_buffer, "read", 4) == 0){
		cmd_readfile(input_buffer);
	} else if (strncmp(input_buffer, "run", 3) == 0){
		cmd_run(input_buffer);
	} else if (strncmp(input_buffer, "edit", 4) == 0){
		cmd_edit(input_buffer);
	} else if (strncmp(input_buffer, "color", 5) == 0){
		cmd_color(input_buffer);
	}
	else {
		print(unknown_response);
	}
}
