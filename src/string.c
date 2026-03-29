#include <utilities.h>
#include <memory.h>
int strcmp(const char* s1, const char* s2){
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
	if(str == NULL){
		print("STRLEN RECEIVED NULL\n");
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

char* strcpy(char* dst, const char* src){
	size_t len = strlen(src);
	for(size_t i = 0; i < len; i++){
		dst[i] = *src;
		src++;
	}
	dst[len] = '\0';
	return dst;
}

char* strdup(const char* src){
	//malloc strlen
	//memcpy string data
	//return pointer to malloc
	size_t size = strlen(src) + 1;
	char* dupData = (char*)malloc(size);
	memcpy(dupData, (void*)src, size);
	return dupData;
}

char tolower(const char c){
	if(c >= 65 && c <=90){ //A and Z
		return c + 32;
	}
	return c; //return same character if not capital letter
}
