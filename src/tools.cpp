#include "utilities.hpp"
#include "string.hpp"



void calc(const char* buf){ //assuming input is calc 3+4 no whitespace 
	int x = buf[5] - '0';
	//char op = buf[6]; need to implement shift
	char op = '+';
	int y = buf[7] - '0';
	
	switch(op){
		case('+'): {
			int sum = [](int a, int b) {return a + b;}(x,y);
			print_num(sum);
			break;
		}
		default:
			print("Usage calc x+y note whitespace");
			break;
	}
}
