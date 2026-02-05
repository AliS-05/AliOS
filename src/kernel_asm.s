bits 32
extern kernel_main
extern parse_command
global kernel
extern print
global init_screen
global skip_newline
kernel:
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov ss, ax
	mov fs, ax
	mov gs, ax
	mov esp, 0x90000

	cli
	call remap_pic
	call init_screen
	sti
	
	;prints command prompt
	call kernel_main
	jmp $

remap_pic:
	

	; Remap PIC
	mov al, 0x11
	out 0x20, al
	out 0xA0, al

	mov al, 0x20 
	out 0x21, al
	mov al, 0x28 
	out 0xA1, al

	mov al, 0x04
	out 0x21, al
	mov al, 0x02
	out 0xA1, al

	mov al, 0x01
	out 0x21, al
	out 0xA1, al

	; Enable Keyboard IRQ only
	mov al, 0xFD 
	out 0x21, al 
	mov al, 0xFF 
	out 0xA1, al

	lidt [idtr]
	ret


idt_start:
    times 32 dq 0 ; Exceptions
    ; int 0x20 timer
    dw timer_handler, 0x08
    db 0, 10001110b
    dw 0
    ; int 0x21 keyboard
    dw keyboard_handler, 0x08
    db 0, 10001110b
    dw 0
    times (256-34) dq 0
idt_end:

idtr:
    dw idt_end - idt_start - 1
    dd idt_start

timer_handler:
    push eax
    mov al, 0x20
    out 0x20, al
    pop eax
    iretd

keyboard_handler:
	pushad
	cld
	mov ax, 0x10
	mov ds, ax
	movzx edi, word [cursor_pos] ;saving cursor pos in register

	in al, 0x60 ; reading scancode
	test al, 0x80 ; if scancode is a relase we skip
	jnz .check_release ; NOTE need to implement key release for shift and ctrl

	cmp al, 0x2A ;left shift
	je .shift_press

	cmp al, 0x36 ;right shift
	je .shift_press

	cmp al, 0x1D ;left ctrl
	je .ctrl_press

	cmp al, 0x0E ;backspace
	je .handle_backspace

	cmp al, 0x1C ;enter
	je .handle_enter
	
	;cmp edi, 160 ;end of line
	;je .done

	movzx ebx, al ;padding scancode in ebx register

	;NOTE also disables space... need to work around this.
	;test al, al ; checks if printable character ie not a character defined as 0 in the scancode table
	;jz .done

	cmp byte [shift_pressed], 1
	je .use_shifted

	mov al, [scancode_table + ebx] ; finding actual character
	jmp .got_char
.use_shifted:
	mov byte al, [scancode_table_shifted + ebx]
.got_char:
	mov byte [0xB8000 + edi], al
	mov byte [0xB8001 + edi], 0x0F 

	add word [cursor_pos], 2 ;moving to next character

	movzx edi, word [buffer_pos] ; writing to buffer
	inc word [buffer_pos]
	mov [input_buffer + edi], al ;COMMAND BUFFER
	jmp .done

.check_release:
	cmp al, 0xAA; left shift release
	je .shift_release 
	cmp al, 0xB6;right shift release
	je .shift_release 
	
	cmp al, 0x9D
	je .ctrl_release
	jmp .done

.shift_press:
	mov byte [shift_pressed], 1
	jmp .done

.shift_release:
	mov byte [shift_pressed], 0
	jmp .done

.ctrl_press:
	mov byte [ctrl_pressed], 1
	jmp .done

.ctrl_release:
	mov byte [ctrl_pressed], 0
	jmp .done


.done:
    mov al, 0x20 ;telling pic we received the message
    out 0x20, al
    popad
    iretd

.handle_backspace:
	cmp edi, 0
	je .done ;at 0,0 theres nowhere to go
	;adjust buffer
	dec word [buffer_pos] ;
	sub word [cursor_pos], 2 ;because we're always infront of the previous character
	movzx edi, word [cursor_pos];

	mov byte [0xB8000 + edi], ' '
	mov byte [0xB8001 + edi], 0x00
	
	
	;mov [input_buffer + edi], byte 0 ;replacing with 0
	jmp .done

.handle_enter:
	movzx edi, word [buffer_pos] ;end of buffer
	mov [input_buffer + edi], byte 0 ; null terminate buffer
	
	cmp word [buffer_pos], 0 ;empty new line if no command and press enter
	je .empty_line

	call .newline

	call parse_command

	cmp byte [skip_newline], 1
	je .skip_nl

	call .newline

	mov word [buffer_pos], 0 ;resetting buffer

	push shell_prompt
	call print
	add esp, 4
	jmp .done

.skip_nl:
	mov byte [skip_newline], 0
	mov word [buffer_pos] , 0
	push shell_prompt
	call print
	add esp, 4

	jmp .done

.empty_line:
	call .newline
	push shell_prompt
	call print
	add esp, 4
	jmp .done
.newline:
	xor dx, dx
	mov ax, [cursor_pos]
	mov bx, 160
	div bx
	inc ax
	mul bx
	mov word [cursor_pos], ax
	ret






init_screen:
	pushad

	xor ebx, ebx
	mov ecx, 2000
.draw:
	mov byte [0xB8000 + ebx], ' '
	mov byte [0xB8001 + ebx], 0x00 ;black
	add ebx, byte 2
	loop .draw
	popad
	ret

section .bss
	input_buffer resb 80 ;reserve 80 bytes for user inputs (line length)
	command_buffer resb 10 ; 10 character command should be more than enough

section .data
	cursor_pos dw 0
	buffer_pos dw 0
	skip_newline db 0	

	;commands
	cmd_help db "help", 0
	cmd_clear db "clear", 0
	cmd_reboot db "reboot", 0
	
	;responses
	shell_prompt db "Enter command -> ", 0 ;null terminated string
	shell_prompt_len equ $-shell_prompt
	help_response db "Supported Commands: clear, reboot, echo, calc", 0
	unknown_response db "Unknown Command. Try typing 'help'", 0

	shift_pressed db 0
	ctrl_pressed db 0

	scancode_table:
		db 0, 27, '1' , '2' , '3' , '4' , '5' , '6' , '7' , '8' , '9' , '0' , '-' , '='
		db 0x08, 0x09, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']'
		db 13, 0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', "'", '`'
		db 0, '\', 'z','x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0
		db 0, 0, ' ', 0 ; 0x39 = spacebar

	scancode_table_shifted:
		db 0, 27, '!' , '@' , '#' , '$' , '%' , '^' , '&' , '*' , '(' , ')' , '_' , '+'
		db 0x08, 0x09, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}'
		db 13, 0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~'
		db 0, '|', 'Z','X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0
		db 0, 0, ' '



global cursor_pos
global buffer_pos
global shell_prompt
global input_buffer
global help_response
global unknown_response
global shift_pressed
global ctrl_pressed
