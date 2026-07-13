#ifndef EDIT_H
#define EDIT_H


extern volatile uint8_t vga_color;
extern volatile uint8_t in_editor;
extern volatile uint8_t esc_pressed;
extern volatile uint8_t enter_editor_flag;
extern volatile uint8_t editor_scancode;
extern volatile uint8_t editor_mode; // normal, insert, command

void save_editor_content(const char* filename, const char* extension);
uint8_t getkey();
void edit_loop(const char* filename, const char* extension);

#endif
