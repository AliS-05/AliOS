#include "utilities.hpp"
#include "string.hpp"

void calc(char* buf){ 
    token(buf, ' ');

    const char *x = token(NULL, ' ');
    int realx = atoi(x);

    const char* op = token(NULL, ' ');

    char *y = token(NULL, ' ');
    int realy = atoi(y);

    if(strcmp(op, "+") == 0){
        auto sum = [](int a, int b){return a+b;};
        print_num((int)sum(realx,realy));
    } else if (strcmp(op, "-") == 0){
	    auto res = [](int a, int b){return a - b;};
	    print_num((int)res(realx,realy));
    } else if (strcmp(op, "*") == 0){
	    auto res = [](int a, int b){return a * b;};
	    print_num((int)res(realx,realy));
    } else if (strcmp(op, "/") == 0){
	    auto res = [](int a, int b){ return a / b; };
	    print_num((int)res(realx,realy));
    } else{
        print("Usage: calc x + y");
    }
}


void hexdump(void* mem, size_t len){ 
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
