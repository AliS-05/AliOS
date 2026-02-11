#include "structures.hpp"
#include "utilities.hpp"
#include "string.hpp"
#include "memory.hpp"


//color reference 

//0x0 Black          0x8 Dark Gray
//0x1 Blue           0x9 Light Blue
//0x2 Green          0xA Light Green
//0x3 Cyan           0xB Light Cyan
//0x4 Red            0xC Light Red
//0x5 Magenta        0xD Light Magenta
//0x6 Brown          0xE Yellow
//0x7 Light Gray     0xF White

void init_editor_screen(uint8_t colorByte){
	cursor_pos = 0;
	unsigned char* vga = (unsigned char*)0xB8000;
	for(int i = 0; i < 2000; i++){
		vga[cursor_pos] = ' ';
		vga[cursor_pos+1] = colorByte;
		cursor_pos += 2;
	}
	cursor_pos = 0;
}

void edit_loop(){
	init_editor_screen(0xFA);
}
