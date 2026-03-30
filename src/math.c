#include <structures.h>

int ceil(float x){
	int i = (int)x;
	if(x > 0 && x != (float)i){
		return i + 1;
	}
	return i;
}
