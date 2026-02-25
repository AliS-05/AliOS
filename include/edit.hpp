#ifndef EDIT_H
#define EDIT_H


extern "C" volatile uint8_t vga_color;
extern "C" volatile uint8_t in_editor;
extern "C" volatile uint8_t esc_pressed;
extern "C" volatile uint8_t enter_editor_flag;
extern "C" volatile uint8_t editor_scancode;
extern "C" volatile uint8_t editor_mode; // normal, insert, command

void save_editor_content(const char* filename);
uint8_t getkey();
extern "C" void edit_loop(const char* filename);

#endif
