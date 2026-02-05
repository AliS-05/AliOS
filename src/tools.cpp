#include "utilities.hpp"
#include "string.hpp"



void calc(char* buf){ //assuming input is calc 3+4 no whitespace 
	token(buf, ' ');

	//char xbuf[32];
	//xbuf[0] = '\0';

	//char ybuf[32];
	//ybuf[0] = '\0';

	const char *x = token(buf, ' ');
	print("First token: \n");
	print_char(*x);
	int realx = atoi(x);
	print("Converted to: ");
	print_num(realx);

	const char* op = token(buf, ' ');
	print_char(*op);

	char *y = token(buf, ' ');
	int realy = atoi(y);
	print_num(realy);

	if(strcmp(op, (const char*)'+') == 0){
		int sum = [](int a, int b) {return a + b;}(realx,realy);
		print_num(sum);
	}
	else{
		print("Usage calc x+y note whitespace");
	}
}
