#include "utilities.hpp"

extern "C" int strcmp(const char* s1, const char* s2){
	while (*s1 && *s2 && (*s1 == *s2)){
		s1++;
		s2++;
	}
	return (unsigned char)*s1 - (unsigned char)*s2; // how different each string is ie non-zero means no match
}


int strncmp(const char* s1, const char* s2, size_t n) {
	while (n > 0 && *s1 && (*s1 == *s2)) {
		s1++;
		s2++;
		n--;
	}
	if (n == 0) return 0;
	return (unsigned char)*s1 - (unsigned char)*s2;
}

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
