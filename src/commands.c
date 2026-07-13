#include <structures.h>
#include <memory.h>
#include <utilities.h>
#include <string.h>
#include <tools.h>
#include <fs.h>
#include <fat16.h>
#include <edit.h>

extern volatile uint8_t enter_editor_flag;
extern char terminalScrollBuffer[1000][80];
extern program_running;
char* global_source;


void cmd_help() {
	print("AliOS v1.0 - Available Commands:\n");
	print("Use CTRL-U and CTRL-D to scroll up and down respectively");
	print("FILESYSTEM: **NOTE** you MUST pad filenames to 8 characters with spaces\n");
	print("  ls                    - List files\n");
	print("  write <file.ext> data - Create file with data\n");
	print("  read <file.ext> [n]   - Read file (n bytes, default 25)\n");
	print("  del <file.ext>        - Delete file\n");
	print("  edit <file.ext>       - Open text editor\n\n");

	print("EXECUTION: **AGAIN** you MUST pad filenames, run snake<SPACE><SPACE><SPACE>.bin\n");
	print("  run <file.ext>        - Execute binary file\n");
	print("  assemble <file.asm>   - Assemble x86 code to asoutput.exe\n\n");

	print("UTILITIES:\n");
	print("  echo <text>           - Print text\n");
	print("  calc <expression>     - Simple calculator\n");
	print("  hexdump <addr> [size] - Dump memory (default 0x100000, 256)\n");
	print("  color <num>           - Change screen color (0-255)\n");
	print("  clear                 - Clear screen\n");
	print("  reboot                - Restart system\n\n");

	print("EDITOR: i=insert, ESC=normal, :wq=save+quit, :q=quit, hjkl=move\n");
	print("  edit <filename>       - Open editor on file\n");
}

void cmd_clear() {
	memset(terminalScrollBuffer, 0, sizeof(terminalScrollBuffer));
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
		if(strlen(arg2) < 1){ //ie no arg2 -> default value 256
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
	listfiles_fat16();
}

void cmd_makefile(char* input_buffer){
	//expected input something like
	//write test.txt hello world!

	token(input_buffer, ' ');//write

	const char* filename = token(NULL, '.'); //test
	const char* extension = token(NULL, ' '); //txt

	if(filename == NULL){
		print("Error finding file to delete");
		return;
	}

	if(!extension){
		print("Please provide the file extension\n");
		return;
	}

	//inputbuffer offset since token null terminates after each token
	// + 1 to skip null terminator
	const char* data = input_buffer + strlen("write\0") + 1 + strlen(filename) + 1 + strlen(extension) + 1;//hello

	writeFile(filename, extension, (uint8_t*)data, strlen(data));
	return;
}

void cmd_delfile(char* input_buffer){
	//del test.txt
	token(input_buffer, ' ');
	const char* filename = token(NULL, '.');
	const char* extension = token(NULL, ' ');

	if(!filename || !extension){
		print("Error finding file to delete");
		return;
	}
	deleteFile(filename, extension);
}

void cmd_readfile(char* input_buffer){
	token(input_buffer, ' ');
	const char* filename = token(NULL, '.');
	const char* extension = token(NULL, ' ');
	if(filename == NULL || extension == NULL){
		print("Error finding file to read");
		return;
	}

	//read will just print 25 bytes as a default ig
	const char* size = token(NULL, ' ');

	uint8_t* fileData = readFile(filename, extension);
	int requestedSize = atoi(size);
	if(requestedSize == 0){
		for(uint8_t i = 0; i < 25; i++){
			print_char((unsigned char)fileData[i]);
		}
	} else{
		uint32_t fsize = getFileSize(filename, extension);
		for(uint32_t i = 0; i < requestedSize && i < fsize; i++){ //making sure not to read garbage data
			print_char((unsigned char)fileData[i]);
		}
	}
	free((void*)fileData);
	return;
}


void cmd_assemble(char* input_buffer){
	token(input_buffer, ' '); // "assemble"
	const char* input_file = token(NULL, '.');
	const char* input_ext = token(NULL, ' ');

	if(!input_file || !input_ext){
		print("Usage: assemble <input.asm>\n");
		return;
	}

	char* source = (char*)readFile(input_file, input_ext);
	print(source);
	if(!source){
		print("File not found\n");
		return;
	}

	extern void assemble_buffer(char* buffer);
	assemble_buffer(source);

	free(source);
}

void cmd_run(char* input_buffer){
	token(input_buffer, ' ');//run
	const char* filename = token(NULL, '.'); //asoutput
	const char* extension = token(NULL, ' ');//exe

	if(!filename || !extension){
		print("File not found\n");
		return;
	}

	uint8_t* fileData = readFile(filename, extension);
	if(fileData == NULL){
		print("ERROR RUNNING FILEDATA NULL\n");
	}

	uint8_t* memory = (uint8_t*)0x200000;
	memcpy(memory, fileData, getFileSize(filename, extension));

	//print_num((uint32_t)memory);

	typedef int (*Program)();
	Program program = (Program)memory; //cast typedef'd function pointer to malloc memory and then call it
	program_running = 1;
	int ret = program();
	free(fileData);
	program_running = 0;
	print("Program Finished: ");
	print_num(ret);
}

void cmd_edit(char* input_buffer){
	token(input_buffer, ' '); //edit
	const char* filename =  token(NULL, '.'); //test.
	const char* extension = token(NULL, ' ');
	if(!filename || !extension){
		print("Usage edit <filename>.<extension>");
	}
	extern char editor_filename[8];
	extern char editor_extension[3];
	strcpy(editor_filename, filename);
	strcpy(editor_extension, extension);
	//editor gets called from kernel_asm.s file for some reason
	//i tried changing it but if it ain't broke don't fix it
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

void getDate(){
	//RTC hardware register stuff...
	//PDF downloaded implement later
}

void getTime(){

}

void parse_command() {
	
	if (strcmp(input_buffer, "help") == 0) {
		cmd_help();
	} else if (strcmp(input_buffer, "clear") == 0) {
		cmd_clear();
	} else if (strcmp(input_buffer, "reboot") == 0) {
		cmd_reboot();
	} else if (strncmp(input_buffer, "echo", 4) == 0) {
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
	} else if (strncmp(input_buffer, "assemble", 8) == 0){
		cmd_assemble(input_buffer);
	}
	else {
		print(unknown_response);
	}
}

