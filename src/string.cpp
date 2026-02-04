#include "utilities.hpp"
//prob going to need malloc as well for better string methods but i dont think my malloc is quite there yet


size_t strlen(const char* str){
	if(str == nullptr){
		print("CRITICAL ERROR STRLEN RECEIVED NULLPTR\n");
		return 0;
	}
	size_t count = 0;
	while(*str != '\0'){
		count++;
		str++;
	}
	return count;
}

char* strcat(char* dst, const char* src){ 
	size_t len1 = strlen(dst);
	size_t len2 = strlen(src);

	for(size_t i = len1; i < len1 + len2; i++){
		dst[i] = *src;
		src++;
	}
	dst[len1+len2] = '\0';
	return dst;
}
