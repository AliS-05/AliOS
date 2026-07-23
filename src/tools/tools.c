#include <utilities.h>
#include <string.h>

void calc(char* buf){ 
	token(buf, ' ');

	const char *strx = token(NULL, ' ');
	int x = atoi(strx);

	const char* op = token(NULL, ' ');

	char *stry = token(NULL, ' ');
	int y = atoi(stry);
	char prbuf[32];
	if(strcmp(op, "+") == 0){
		print(ntos(x + y, prbuf, 10));
	} else if (strcmp(op, "-") == 0){
		print(ntos(x - y, prbuf, 10));
	} else if (strcmp(op, "*") == 0){
		print(ntos(x * y, prbuf, 10));
	} else if (strcmp(op, "/") == 0){
		print(ntos(x / y, prbuf, 10));
	} else{
		print("Usage: calc x + y");
	}
}


void hexdump(void* mem, size_t len){ 
	// rereading this i have no idea what this means i think i wanted it to print the hex AND ascii values
	// goal :"0x100000: 0000 0000 0000 0000 0000 0000 0000 0000 -> Ascii: '1234567812345678'\n"	

	uintptr_t addr = (uintptr_t)mem; // numerical value of memory location passed
	unsigned char* val = (unsigned char*)mem;
	print_addr(addr); //printing the numerical value
	print(": ");
	for(uintptr_t i = addr; i < addr+len; i++){
		print_byte(*val);
		val++;
	}

}
