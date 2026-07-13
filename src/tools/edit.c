#include <structures.h>
#include <utilities.h>
#include <string.h>
#include <memory.h>
#include <fs.h>

//color reference

//0x0 Black		0x8 Dark Gray
//0x1 Blue		 0x9 Light Blue
//0x2 Green		0xA Light Green
//0x3 Cyan		 0xB Light Cyan
//0x4 Red		  0xC Light Red
//0x5 Magenta	   0xD Light Magenta
//0x6 Brown		0xE Yellow
//0x7 Light Gray	0xF White

extern volatile uint8_t vga_color;
extern volatile uint8_t in_editor;
extern volatile uint8_t esc_pressed;
extern volatile uint8_t enter_editor_flag;
extern volatile uint8_t editor_scancode;
extern volatile uint8_t editor_mode; // normal, insert, command

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

// Editor is only as tall as what we actually draw (no scrolling yet), so
// keep the buffer, the redraw, the save and the load all on the same height.
#define EDITOR_ROWS 24
#define EDITOR_COLS 80
#define CLUSTER_BYTES 2048

// writeFile always flushes whole clusters from the source buffer, so the save
// buffer has to be rounded up to a full cluster or the FS reads past its end.
#define RAW_BYTES (EDITOR_ROWS * (EDITOR_COLS + 1))
#define SAVE_BUFFER_SIZE (((RAW_BYTES + CLUSTER_BYTES - 1) / CLUSTER_BYTES) * CLUSTER_BYTES)

char lines[EDITOR_ROWS][EDITOR_COLS];
int cursor_row = 0, cursor_col = 0, num_lines = 1;
char cmd[80];
int cmd_len = 0;
volatile uint8_t editor_char = 0;


// Length of a row's content = index just past the last non-empty cell.
// Empty cells are stored as 0 (drawn as a space), so trim trailing 0/space.
int line_length(int row){
	int len = EDITOR_COLS;
	while(len > 0 && (lines[row][len - 1] == 0 || lines[row][len - 1] == ' ')){
		len--;
	}
	return len;
}


void save_editor_content(const char* filename, const char* extension) {
	static uint8_t save_buffer[SAVE_BUFFER_SIZE];
	memset(save_buffer, 0, sizeof(save_buffer));
	int write_pos = 0;

	// Newline-delimited, one '\n' per line (including blank ones) so the exact
	// line layout round-trips through load. Trailing padding is trimmed.
	for (int r = 0; r < num_lines; r++) {
		int len = line_length(r);
		for (int c = 0; c < len; c++) {
			save_buffer[write_pos++] = lines[r][c];
		}
		save_buffer[write_pos++] = '\n';
	}

	deleteFile(filename, extension);
	if (write_pos > 0) {
		writeFile(filename, extension, save_buffer, write_pos);
	}
}

uint8_t getkey() {
	editor_scancode = 0;
	while(!editor_scancode){
		__asm__("sti");
		__asm__("hlt");
	}
	uint8_t k = editor_scancode;
	editor_scancode = 0;
	return k;
}


void redraw(uint8_t color) {
    unsigned char* vga = (unsigned char*)0xB8000;
    for(int i = 0; i < EDITOR_ROWS; i++) {
        for(int j = 0; j < EDITOR_COLS; j++) {
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

extern void edit_loop(const char* filename, const char* extension) {
	in_editor = 0;
	editor_scancode = 0;
	editor_mode = 0;

	init_editor_screen(0x0E);

	memset(lines, 0, sizeof(lines));
	num_lines = 1;

	// Load: parse the newline-delimited format save_editor_content writes back
	// into the 2D grid. getFileSize gives the real length so we never read past
	// the malloc'd buffer.
	uint8_t* load_buffer = readFile(filename, extension);
	if(load_buffer != NULL) {
		uint32_t size = getFileSize(filename, extension);
		int row = 0, col = 0;
		for(uint32_t i = 0; i < size && row < EDITOR_ROWS; i++){
			char ch = (char)load_buffer[i];
			if(ch == '\n'){
				row++;
				col = 0;
			} else if(ch != 0){
				if(col < EDITOR_COLS){
					lines[row][col++] = ch;
				}
			}
		}
		num_lines = row + ((col > 0) ? 1 : 0);
		if(num_lines < 1) num_lines = 1;
		if(num_lines > EDITOR_ROWS) num_lines = EDITOR_ROWS;
		free(load_buffer);
	}

	cursor_row = (num_lines > 0) ? num_lines - 1 : 0;
	cursor_col = line_length(cursor_row);
	if(cursor_col >= EDITOR_COLS) cursor_col = EDITOR_COLS - 1;


	in_editor = 1;
	init_editor_screen(0x0F);

	while(in_editor) {
	   redraw(vga_color);
	   uint8_t k = getkey();
	   if(k & 0x80) continue;  // Release

	   if(editor_mode == MODE_NORMAL) {

		  if(k == KEY_H && cursor_col > 0) cursor_col--;
		  else if(k == KEY_J && cursor_row < num_lines-1) cursor_row++;
		  else if(k == KEY_K && cursor_row > 0) cursor_row--;
		  else if(k == KEY_L && cursor_col < EDITOR_COLS-1) cursor_col++;
		  else if(k == KEY_I) editor_mode = MODE_INSERT;
		  else if(k == KEY_COLON) { editor_mode = MODE_COMMAND; cmd_len = 0; }
	   }
	   else if(editor_mode == MODE_INSERT) {
		  if(k == KEY_ESC) editor_mode = MODE_NORMAL;
		  else if(k == KEY_ENTER) {
			  // Split the current line at the cursor and push the rest down.
			  if(num_lines < EDITOR_ROWS){
				  for(int r = num_lines; r > cursor_row + 1; r--){
					  memcpy(lines[r], lines[r-1], EDITOR_COLS);
				  }
				  memset(lines[cursor_row + 1], 0, EDITOR_COLS);
				  int len = line_length(cursor_row);
				  for(int c = cursor_col; c < len; c++){
					  lines[cursor_row + 1][c - cursor_col] = lines[cursor_row][c];
				  }
				  for(int c = cursor_col; c < EDITOR_COLS; c++){
					  lines[cursor_row][c] = 0;
				  }
				  num_lines++;
				  cursor_row++;
				  cursor_col = 0;
			  }
		  }
		  else if(k == KEY_BACKSPACE){
			  if(cursor_col > 0){
				  // Delete char before cursor, shift the tail left.
				  char* line = lines[cursor_row];
				  for(int i = cursor_col - 1; i < EDITOR_COLS - 1; i++){
					  line[i] = line[i+1];
				  }
				  line[EDITOR_COLS - 1] = 0;
				  cursor_col--;
			  } else if(cursor_row > 0){
				  // At column 0: join this line onto the end of the previous one.
				  int prev = cursor_row - 1;
				  int plen = line_length(prev);
				  int clen = line_length(cursor_row);
				  int join_col = plen;
				  for(int c = 0; c < clen && plen + c < EDITOR_COLS; c++){
					  lines[prev][plen + c] = lines[cursor_row][c];
				  }
				  for(int r = cursor_row; r < num_lines - 1; r++){
					  memcpy(lines[r], lines[r+1], EDITOR_COLS);
				  }
				  memset(lines[num_lines - 1], 0, EDITOR_COLS);
				  num_lines--;
				  cursor_row = prev;
				  cursor_col = join_col;
			  }
		  }
		  else {
			  // Printable: insert at cursor, shifting the tail right (last col drops).
			  char c = editor_char;
			  if(c != 0){
				  char* line = lines[cursor_row];
				  for(int i = EDITOR_COLS - 1; i > cursor_col; i--){
					  line[i] = line[i-1];
				  }
				  line[cursor_col] = c;
				  if(cursor_col < EDITOR_COLS - 1) cursor_col++;
			  }
		  }
	   }
	   else {  // COMMAND
		  if(k == KEY_ESC) editor_mode = MODE_NORMAL;
		  else if(k == KEY_ENTER) {
			 cmd[cmd_len] = 0;
			 if(strcmp(cmd, "wq") == 0) {
				save_editor_content(filename, extension);
				in_editor = 0;
			 }
			 else if(strcmp(cmd, "q") == 0 || strcmp(cmd, "q!") == 0) {
				in_editor = 0;
			 }
			 else if(strcmp(cmd, "w") == 0) {
				save_editor_content(filename, extension);
			 }
			 editor_mode = MODE_NORMAL;
		  }
		  else if(k == KEY_BACKSPACE && cmd_len > 0) cmd_len--;
		  else { char c = editor_char; if(c != 0 && cmd_len < (int)sizeof(cmd) - 1) cmd[cmd_len++] = c; }
	   }
	}

	in_editor = 0;
	init_editor_screen(vga_color);
}
