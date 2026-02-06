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
        int sum = realx + realy;
        print_num(sum);
    }
    else{
        print("Usage: calc x + y");
    }
}

