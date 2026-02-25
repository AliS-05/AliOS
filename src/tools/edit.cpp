#include <structures.hpp>
#include <utilities.hpp>
#include <string.hpp>
#include <memory.hpp>
#include <fs.hpp>

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



char lines[50][80];
int cursor_row = 0, cursor_col = 0, num_lines = 1;
char cmd[80];
int cmd_len = 0;

void save_editor_content(const char* filename) {
    // 50 lines * 80 chars = 4000 bytes
    uint8_t save_buffer[4000]; 
    
    // Flatten the 2D 'lines' array into 'save_buffer'
    for (int r = 0; r < 50; r++) {
        for (int c = 0; c < 80; c++) {
            save_buffer[r * 80 + c] = (uint8_t)lines[r][c];
        }
    }

    overwrite_file(filename, save_buffer, 4000);
}

uint8_t getkey() {
	editor_scancode = 0;
	while(!editor_scancode){
		asm("sti");
		asm("hlt");
	}
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

void redraw(uint8_t color) {
    unsigned char* vga = (unsigned char*)0xB8000;
    for(int i = 0; i < 24; i++) {
        for(int j = 0; j < 80; j++) {
            int vga_pos = (i*160) + j*2;
            vga[vga_pos] = (i < num_lines) ? (lines[i][j] ?: ' ') : ' ';
            
            // Highlight cursor position in normal mode
            if(i == cursor_row && j == cursor_col && editor_mode == 0) {
                vga[vga_pos + 1] = 0xF0;  // Inverted colors (white bg, black text)
            } else {
                vga[vga_pos + 1] = color;
            }
        }
    }
    cursor_pos = cursor_row * 160 + cursor_col * 2;
}

extern "C" void edit_loop(const char* filename) {
	in_editor = 0;
	editor_scancode = 0;
	editor_mode = 0;

	init_editor_screen(0x0E);

	memset(lines, 0, sizeof(lines));
	

	static uint8_t load_buffer[4000];
	if(cpy_file_buffer(filename, load_buffer, 4000) != NULL) {
		// Unflatten the buffer back into the 2D lines array
		for (int r = 0; r < 50; r++) {
		    boolean line_has_data = false;
		    for (int c = 0; c < 80; c++) {
			char val = load_buffer[r * 80 + c];
			lines[r][c] = val;

			if(val != 0 && val != ' '){
				line_has_data = true;
			}
		    }
		    if(line_has_data){
			    num_lines = r + 1;
		    }
		}
	}

	cursor_row = (num_lines > 0) ? num_lines - 1: 0;
	cursor_col = strlen(lines[cursor_row]);

	
     in_editor = 1;
    init_editor_screen(0x0F);

	while(in_editor) {
	   redraw(vga_color);
	   uint8_t k = getkey();
	   if(k & 0x80) continue;  // Release
	   
	   if(editor_mode == 0) {  // NORMAL
		
		  if(k == KEY_H && cursor_col > 0) cursor_col--;		
		  else if(k == KEY_J && cursor_row < num_lines-1) cursor_row++;
		  else if(k == KEY_K && cursor_row > 0) cursor_row--;		
		  else if(k == KEY_L && cursor_col < 79) cursor_col++;	
		  else if(k == KEY_I) editor_mode = 1;				
		  else if(k == KEY_COLON) { editor_mode = 2; cmd_len = 0; }	 
		  vga_color = 0xF0;
		  print_char(' ');
		  vga_color = 0x0F;
	   }
	   else if(editor_mode == 1) {  // INSERT
		  if(k == KEY_ESC) editor_mode = 0;						
		  else if(k == KEY_ENTER) { cursor_row++; cursor_col = 0; if(cursor_row >= num_lines) num_lines++; }
		  else if(k == KEY_BACKSPACE && cursor_col > 0){
			  char* line = lines[cursor_row];
			  int len = strlen(line);
			  for(int i = cursor_col - 1; i < len; i++){
				  line[i] = line[i+1];
			  }
			  cursor_col--;		
		  }
		  else { char c = sc2char(k); if(c) lines[cursor_row][cursor_col++] = c; }
	   }
	   else {  // COMMAND
		  if(k == KEY_ESC) editor_mode = 0;						
		  else if(k == KEY_ENTER) {							  
			 cmd[cmd_len] = 0;
			 if(strcmp(cmd, "wq") == 0) {
				save_editor_content(filename); 
				in_editor = 0;
			 } 
			 else if(strcmp(cmd, "q") == 0 || strcmp(cmd, "q!") == 0) {
				in_editor = 0;
			 }
			 editor_mode = 0;
		  }
		  else if(k == 0x0E && cmd_len > 0) cmd_len--;			// Backspace
		  else { char c = sc2char(k); if(c) cmd[cmd_len++] = c; }
	   }
	}
	
	in_editor = 0;
	init_editor_screen(vga_color);
}

