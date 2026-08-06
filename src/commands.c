#include <core/structures.h>
#include <core/memory.h>
#include <core/utilities.h>
#include <core/string.h>
#include <core/tools.h>
#include <fs/fs.h>
#include <fs/fat16.h>
#include <core/edit.h>
#include <networking/packetHandling.h>
#include <networking/icmp.h>
#include <networking/dns.h>
#include <networking/tftp.h>

char commandHistoryBuffer[50][80] = {0}; //stores last 50 commands;
int currentCommandAnchor = 0;
int writeIndexAnchor = 0;

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
	listfiles_dir();
}


void cmd_makefile(char* input_buffer){
	//write /games/test    .txt hello world!
	token(input_buffer, ' ');

	char* path = token(NULL, '.');
	char* extension = token(NULL, ' ');
	const char* data = extension + strlen(extension) + 1;

	int saved = currentCluster;
	char* filename = parsePath(path);
	if(!filename){
		print("Path not found\n");
		currentCluster = saved;
		return;
	}

	writeFile(filename, extension, (uint8_t*)data, strlen(data));
	currentCluster = saved;
}



void cmd_delfile(char* input_buffer){
	//del /games/test    .txt
	token(input_buffer, ' ');

	char* path = token(NULL, '.');
	char* extension = token(NULL, ' ');

	int saved = currentCluster;
	char* filename = parsePath(path);
	if(!filename || !extension){
		print("Error finding file to delete");
		currentCluster = saved;
		return;
	}

	deleteFile(filename, extension);
	currentCluster = saved;
}

void cmd_readfile(char* input_buffer){
	token(input_buffer, ' ');

	char* path = token(NULL, '.');
	char* extension = token(NULL, ' ');
	const char* size = token(NULL, ' ');

	int saved = currentCluster;
	char* filename = parsePath(path);
	if(!filename || !extension){
		print("Error finding file to read");
		currentCluster = saved;
		return;
	}

	uint8_t* fileData = readFile(filename, extension);
	uint32_t fsize = getFileSize(filename, extension);
	currentCluster = saved;

	if(!fileData){
		print("FILE NOT FOUND\n");
		return;
	}

	int requestedSize = 0;
	if(size)
		requestedSize = atoi(size);
	if(requestedSize == 0){
		for(uint32_t i = 0; i < fsize; i++){
			putChar((unsigned char)fileData[i]);
		}
	} else {
		for(uint32_t i = 0; i < requestedSize && i < fsize; i++){
			putChar((unsigned char)fileData[i]);
		}
	}
	free((void*)fileData);
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

	//ntos((uint32_t)memory);

	typedef int (*Program)();
	Program program = (Program)memory; //cast typedef'd function pointer to malloc memory and then call it
	program_running = 1;
	int ret = program();
	free(fileData);
	program_running = 0;
	print("Program Finished: ");
	char buf[32];
	ntos(ret,buf,10);
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
	//init_editor_screen((uint8_t)col);
}

void getDate(){
	//RTC hardware register stuff...
	//PDF downloaded implement later
}

void getTime(){

}

void drawLine(char* command){
	//need to redraw line everytime so printing command doesnt add to end of line but replace entirely
	cursor_pos = beginningOfLine(cursor_pos);
	blankLine();
	print(shell_prompt);

	print(commandHistoryBuffer[currentCommandAnchor]);
	strcpy(input_buffer, commandHistoryBuffer[currentCommandAnchor]);
	buffer_pos = strlen(commandHistoryBuffer[currentCommandAnchor]);

}

//prints previously entered command
//0 is first command, 1 is second, 2 is third etc.
// so decrement to print previous commands
void prevCommandHistory(){
	if(currentCommandAnchor == 0) return; // OOB
	currentCommandAnchor--;
	drawLine(commandHistoryBuffer[currentCommandAnchor]);
}

//undoes function above
void nextCommandHistory(){
	if(currentCommandAnchor == 50) return; // OOB
	currentCommandAnchor++;
	drawLine(commandHistoryBuffer[currentCommandAnchor]);
}

void cmd_ping(char* command){
	token(command, ' '); //skipping 'ping'
	const char* first = token(NULL, '.');
	//first is either 'google' or '123'
	if((first[0] > 0x41 && first[0] < 0x5A) || (first[0] > 61 && first[0] < 0x7A)){ //checking for letters
		const char* second = token(NULL, '.'); //com
		size_t len1 = strlen(first);
		size_t len2 = strlen(second);

		size_t len = len1+ len2 + 3;//3 bytes for lengths + null terminator
		uint8_t buf[len]; 
		//0x06
		buf[0] = len1;
		//google
		memcpy(buf + 1, first, len1);
		//0x03
		buf[len1 + 1] = len2;
		//com
		memcpy(buf + len1 + 2, second, len2);
		//0x00
		buf[len1 + 2 + len2] = 0x00;
		ping_dns(buf, len);
	} else {
		//else we have a number between 0-255
		//123.456.789.000
		const char* second = token(NULL, '.');
		const char* third  = token(NULL, '.');
		const char* fourth = token(NULL, ' ');
		
		uint8_t ipEntered[4];
		ipEntered[0] = (uint8_t)atoi(first);
		ipEntered[1] = (uint8_t)atoi(second);
		ipEntered[2] = (uint8_t)atoi(third);
		ipEntered[3] = (uint8_t)atoi(fourth);

		uint32_t destIp = 
			((uint32_t)ipEntered[3] << 24) |
			((uint32_t)ipEntered[2] << 16) |
			((uint32_t)ipEntered[1] << 8) |
			((uint32_t)ipEntered[0]);
		ping(destIp);
	}
}

void cmd_tftp(char* command){
	token(command, ' '); //skipping 'tftp'
	const char* first = token(NULL, '.');
	const char* second = token(NULL, '.');
	const char* third  = token(NULL, '.');
	const char* fourth = token(NULL, ' ');

	uint8_t ipEntered[4];
	ipEntered[0] = (uint8_t)atoi(first);
	ipEntered[1] = (uint8_t)atoi(second);
	ipEntered[2] = (uint8_t)atoi(third);
	ipEntered[3] = (uint8_t)atoi(fourth);

	uint32_t destIp = 
		((uint32_t)ipEntered[3] << 24) |
		((uint32_t)ipEntered[2] << 16) |
		((uint32_t)ipEntered[1] << 8) |
		((uint32_t)ipEntered[0]);
	//input is something like
	// tftp 10.0.2.1 hello.txt
	const char* fileName = token(NULL, ' ');

	tftp_request(destIp, fileName, "octet");

}

void cmd_mkdir(char* command){
	// mkdir games
	token(command, ' ');
	const char* dirname = token(NULL, ' ');
	makeDirectory(dirname);
}

void cmd_cd(char* input_buffer){
	//cd /games/sub     - directory names are not padded, so split on space
	token(input_buffer, ' ');

	char* path = token(NULL, ' ');
	if(!path){
		print("Path not found\n");
		return;
	}

	if(path[0] == '/' && path[1] == 0){     //cd /
		currentCluster = 0;
		return;
	}

	char* filename = parsePath(path);
	if(!filename){
		print("Path not found\n");
		return;
	}

	changeDirectory(filename);
}

void cmd_pwd(){
	printWorkingDirectory();
}

void parse_command() {
	strcpy(commandHistoryBuffer[writeIndexAnchor], input_buffer);
	writeIndexAnchor++;
	currentCommandAnchor = writeIndexAnchor;

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
	} else if (strncmp(input_buffer, "ping", 4) == 0){
		cmd_ping(input_buffer);
	} else if (strncmp(input_buffer, "tftp", 4) == 0){
		cmd_tftp(input_buffer);
	} else if (strncmp(input_buffer, "mkdir", 5) == 0){
		cmd_mkdir(input_buffer);
	} else if (strncmp(input_buffer, "cd", 2) == 0){
		cmd_cd(input_buffer);
	} else if (strncmp(input_buffer, "pwd", 3) == 0){
		cmd_pwd();
	}
	else {
		print(unknown_response);
	}
}

