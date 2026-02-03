
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

void strtonum(int num, char* buf){
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

void print_num(int num){
	char buf[12];
	strtonum(num, buf);
	print_string(buf, cursor_pos);
}

char* heap_start = (char*)0x100000;
char* heap_current=(char*)0x100000;

struct MemoryBlock{
	int size;
	int available; // 0 = available , 1 = in use. basically a bool
	MemoryBlock* next;
	MemoryBlock* prev;
}

void* malloc(int size){
	// first we need to set up a 'header' struct for each block
	MemoryBlock* newBlock = (MemoryBlock*)heap_current; //this puts it directly on the stack
	newBlock->size = size;
	newBlock->available = 1; //in use
	newBlock->next = (MemoryBlock)(heap_current + sizeof(MemoryBlock));
	// how do we implement prev ? calculations ?
	if((int n = (heap_current - sizeof(MemoryBlock))) > 0x100000){
		newBlock->prev = (MemoryBlock)(n);
	}else{
		newBlock->prev = NULL;
	}

	//update heap_current
	heap_current += size + (sizeof(MemoryBlock));


	//and a search function which should run first but well implement that last


	return newBlock;
}



extern "C" void kernel_main(){
	print_string("BOOT OK", 0);
	print_string(shell_prompt, 160);
}
