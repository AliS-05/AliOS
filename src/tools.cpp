#include "utilities.hpp"
#include "string.hpp"



void calc(char* buf){ //assuming input is calc 3+4 no whitespace 
	token(buf, ' ');
	char xbuf[32];
	xbuf[0] = '\0';

	char ybuf[32];
	ybuf[0] = '\0';

	char *x = token(buf, ' ');
	int realx = atoi(x);
	
	char* op = token(buf, ' ');

	char *y = token(buf, ' ');
	int realy = atoi(y);

	if(strcmp(op, (const char*)'+') == 0){
		int sum = [](int a, int b) {return a + b;}(realx,realy);
		print_num(sum);
	}
	else{
		print("Usage calc x+y note whitespace");
	}
}
