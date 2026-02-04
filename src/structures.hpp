#ifndef STRUCTURES_H
#define STRUCTURES_H

#define NULL 0
typedef unsigned int uintptr_t;
typedef signed int intptr_t;
extern "C" {
    extern unsigned short cursor_pos; //2 bytes
    extern unsigned short buffer_pos; //2 bytes
    extern unsigned char skip_newline; //1 byte
    extern char input_buffer[80];
    extern char shell_prompt[];
    extern char help_response[];
    extern char unknown_response[];
    extern void print_string(const char* str, unsigned short pos);
    extern void init_screen();
}



struct MemoryBlock{
	int size;
	int available; // 0 = available , 1 = in use. basically a bool
	MemoryBlock* next;
	MemoryBlock* prev;
};

#endif
