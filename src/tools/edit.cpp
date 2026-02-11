#include <structures.hpp>
#include <utilities.hpp>
#include <string.hpp>
#include <memory.hpp>


//color reference 

//0x0 Black          0x8 Dark Gray
//0x1 Blue           0x9 Light Blue
//0x2 Green          0xA Light Green
//0x3 Cyan           0xB Light Cyan
//0x4 Red            0xC Light Red
//0x5 Magenta        0xD Light Magenta
//0x6 Brown          0xE Yellow
//0x7 Light Gray     0xF White

extern "C" volatile uint8_t vga_color;
extern "C" volatile uint8_t in_editor;
extern "C" volatile uint8_t esc_pressed;
extern "C" volatile uint8_t enter_editor_flag;

void init_editor_screen(uint8_t colorByte){
	vga_color = colorByte;
	int pos = 0;
	unsigned char* vga = (unsigned char*)0xB8000;
	for(int i = 0; i < 2000; i++){
		vga[pos] = ' ';
		vga[pos+1] = colorByte;
		pos += 2;
	}
	cursor_pos = 0;
}

// first digit = background color second digit = text color
extern "C" void edit_loop(){
	esc_pressed = 0;
	in_editor = 1;
	init_editor_screen(0xFA);

	while(!esc_pressed){
		asm volatile("hlt");
	}
	in_editor = 0;
	esc_pressed = 0;
	init_editor_screen(0x0F);
}
