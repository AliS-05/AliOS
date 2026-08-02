#include <core/structures.h>


int max_int(int x, int y) {
    return (x > y) ? x : y;
}

double max_double(double x, double y) {
    return (x > y) ? x : y;
}


int ceil_float(float x){
	int i = (int)x;
	if(x > 0 && x != (float)i){
		return i + 1;
	}
	return i;
}

int ceil_div(int n, int k){
	return (n + k - 1) / k;
}
