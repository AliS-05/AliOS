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

