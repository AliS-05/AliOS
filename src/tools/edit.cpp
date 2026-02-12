#include <structures.hpp>
#include <utilities.hpp>
#include <string.hpp>
#include <memory.hpp>


//color reference 

//0x0 Black		0x8 Dark Gray
//0x1 Blue		 0x9 Light Blue
//0x2 Green		0xA Light Green
//0x3 Cyan		 0xB Light Cyan
//0x4 Red		  0xC Light Red
//0x5 Magenta	   0xD Light Magenta
//0x6 Brown		0xE Yellow
//0x7 Light Gray	0xF White

extern "C" volatile uint8_t vga_color;
extern "C" volatile uint8_t in_editor;
extern "C" volatile uint8_t esc_pressed;
extern "C" volatile uint8_t enter_editor_flag;
extern "C" volatile uint8_t editor_scancode;
extern "C" volatile uint8_t editor_mode; // normal, insert, command


#define MODE_NORMAL  0
#define MODE_INSERT  1
#define MODE_COMMAND 2

// Scancode defines
#define KEY_ESC	 0x01
#define KEY_ENTER	0x1C
#define KEY_BACKSPACE 0x0E
#define KEY_H	   0x23
#define KEY_J	   0x24
#define KEY_K	   0x25
#define KEY_L	   0x26
#define KEY_I	   0x17
#define KEY_COLON	0x27


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

extern "C" volatile uint8_t vga_color;
extern "C" volatile uint8_t in_editor;
extern "C" volatile uint8_t editor_scancode;
extern "C" volatile uint8_t editor_mode;  // 0=normal, 1=insert

char lines[25][80];
int cursor_row = 0, cursor_col = 0, num_lines = 1;
char cmd[80];
int cmd_len = 0;

uint8_t getkey() {
	editor_scancode = 0;
	while(!editor_scancode) asm("hlt");
	uint8_t k = editor_scancode;
	editor_scancode = 0;
	return k;
}

char sc2char(uint8_t sc) {
	const char map[] = {0,0,'1','2','3','4','5','6','7','8','9','0','-','=',
	   0,0,'q','w','e','r','t','y','u','i','o','p','[',']',0,0,
	   'a','s','d','f','g','h','j','k','l',';','\'','`',0,'\\',
	   'z','x','c','v','b','n','m',',','.','/',0,0,0,' '};
	return sc < sizeof(map) ? map[sc] : 0;
}

void redraw() {
	unsigned char* vga = (unsigned char*)0xB8000;
	for(int i = 0; i < 24; i++) {
	   for(int j = 0; j < 80; j++) {
		  vga[(i*160) + j*2] = (i < num_lines) ? (lines[i][j] ?: ' ') : ' ';
		  vga[(i*160) + j*2 + 1] = 0x0F;
	   }
	}
	cursor_pos = cursor_row * 160 + cursor_col * 2;
}

extern "C" void edit_loop(const char* filename) {
	memset(lines, 0, sizeof(lines));
	in_editor = 1;
	editor_mode = 0;
	
	while(in_editor) {
	   redraw();
	   uint8_t k = getkey();
	   if(k & 0x80) continue;  // Release
	   
	   if(editor_mode == 0) {  // NORMAL
		
		  if(k == KEY_H && cursor_col > 0) cursor_col--;			// h
		  else if(k == KEY_J && cursor_row < num_lines-1) cursor_row++; // j
		  else if(k == KEY_K && cursor_row > 0) cursor_row--;		// k
		  else if(k == KEY_L && cursor_col < 79) cursor_col++;		// l
		  else if(k == KEY_I) editor_mode = 1;					// i
		  else if(k == KEY_COLOR) { editor_mode = 2; cmd_len = 0; }	  // :
	   }
	   else if(editor_mode == 1) {  // INSERT
		  if(k == KEY_ESC) editor_mode = 0;						// ESC
		  else if(k == KEY_ENTER) { cursor_row++; cursor_col = 0; if(cursor_row >= num_lines) num_lines++; }
		  else if(k == KEY_BACKSPACE && cursor_col > 0) cursor_col--;		// Backspace
		  else { char c = sc2char(k); if(c) lines[cursor_row][cursor_col++] = c; }
	   }
	   else {  // COMMAND
		  if(k == KEY_ESC) editor_mode = 0;						
		  else if(k == KEY_ENTER) {							  
			 cmd[cmd_len] = 0;
			 if(!strcmp(cmd, "wq") || !strcmp(cmd, "q") || !strcmp(cmd, "q!")) 
				in_editor = 0;
			 editor_mode = 0;
		  }
		  else if(k == 0x0E && cmd_len > 0) cmd_len--;			// Backspace
		  else { char c = sc2char(k); if(c) cmd[cmd_len++] = c; }
	   }
	}
	
	in_editor = 0;
	init_editor_screen(0x0F);
}

