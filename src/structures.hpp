#ifndef STRUCTURES_H
#define STRUCTURES_H

#define NULL 0

typedef unsigned int uintptr_t;
typedef int boolean; 
typedef signed int intptr_t;
typedef unsigned int size_t;

extern "C" {
    extern int cursor_pos; //2 bytes
    extern int buffer_pos; //2 bytes
    extern unsigned char skip_newline; //1 byte
    extern char input_buffer[80];
    extern char shell_prompt[];
    extern char help_response[];
    extern char unknown_response[];
    extern void print(const char* str);
    extern void init_screen();
}


struct MemoryBlock{
	size_t size;
	boolean available; // 0 = available , 1 = in use. basically a bool
	MemoryBlock* next;
	MemoryBlock* prev;
};

#endif
