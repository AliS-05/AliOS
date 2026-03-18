bits 32

extern bss_start
extern bss_end
extern kernel_main
extern parse_command
extern print
extern edit_loop
extern receive_packet


global kernel
global init_screen
global skip_newline
global cursor_pos
global buffer_pos
global shell_prompt
global input_buffer
global help_response
global unknown_response
global shift_pressed
global ctrl_pressed
global vga_color
global in_editor
global esc_pressed
global enter_editor_flag
global editor_scancode
global editor_mode
global editor_filename


kernel:
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov ss, ax
	mov fs, ax
	mov gs, ax
	mov esp, 0x90000

	mov edi, bss_start    ; start address
	mov ecx, bss_end
	sub ecx, edi          ; ecx = size
	xor eax, eax		; AL = 0
	rep stosb             ; write ECX zeros to [EDI]
	lidt [idtr]

	cli
	;call setup_idt
	call remap_pic
	call init_screen
	sti
	
	;prints command prompt
	call kernel_main
	;jmp $
	.idle:
	    sti
	    hlt
	    cmp byte [enter_editor_flag], 1
	    jne .idle
	    mov byte [enter_editor_flag], 0

	    push dword editor_filename
	    call edit_loop
	    add esp, 4
	    ; restore shell after editor exits
	    push shell_prompt
	    call print
	    add esp, 4
	    jmp .idle

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

	ret


idt_start:
    times 32 dq 0 ; Exceptions
    ;int 0x20 timer
    dw timer_handler
    dw 0x08
    db 0, 10001110b
    dw 0x0000
     ;int 0x21 keyboard
    dw keyboard_handler, 0x08
    db 0, 10001110b
    dw 0x0000

    times 9 dq 0 ; skipping to NIC IRQ
    dw nic_handler, 0x08
    db 0, 10001110b
    dw 0x0000

    times (256-24) dq 0

    times 256 dq 0
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

nic_handler:
	pushad
	call receive_packet
	popad
	iretd

keyboard_handler:
	pushad
	;cld
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov edi, [cursor_pos] ;saving cursor pos in register

	in al, 0x60 ; reading scancode
	
	mov byte [editor_scancode], al

	cmp byte [in_editor], 1
	jne .not_in_editor

	mov al, 0x20
	out 0x20, al
	popad
	iretd

	jmp .done

.not_in_editor:
	test al, 0x80 
	jnz .check_release

	cmp al, 0x01 ;escape key
	jne .normal
	mov byte [esc_pressed] , 1
	jmp .done
.normal:
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
	

	movzx ebx, al ;padding scancode in ebx register
	;checking if shift is pressed
	cmp byte [shift_pressed], 1
	je .use_shifted

	mov al, [scancode_table + ebx] ; finding actual character
	jmp .got_char

.use_shifted:
	mov byte al, [scancode_table_shifted + ebx]
.got_char:
	mov byte [0xB8000 + edi], al
	mov byte ah, [vga_color]
	mov byte [0xB8001 + edi], ah

	add dword [cursor_pos], 2 ;moving to next character

	; not filling in buffer if in editor
	cmp byte [in_editor], 1
	je .done

	mov edi, [buffer_pos] ; writing to buffe
	cmp edi, 79 ;leave room for 0 at end of buffer
	jae .done

	mov [input_buffer + edi], al ;COMMAND BUFFER
	inc dword [buffer_pos]

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

	cmp byte [in_editor], 1
	je .editor_backspace
	
	cmp dword [buffer_pos], 0
	je .done

	cmp edi, 0
	je .done ;at 0,0 theres nowhere to go

	;adjust buffer
	dec dword [buffer_pos] ;going back in buffer

	push edi
	mov edi, [buffer_pos] ;adding zero
	mov byte [input_buffer + edi], 0
	pop edi

	sub dword [cursor_pos], 2 ;moving cursor back one
	mov edi, [cursor_pos];

	mov byte [0xB8000 + edi], ' '
	mov byte ah, [vga_color]
	mov byte [0xB8001 + edi], ah
	
	
	;mov [input_buffer + edi], byte 0 ;replacing with 0
	jmp .done

.editor_backspace: ;need special logic for editor backspace
	cmp edi, 0
	je .done

	sub dword [cursor_pos], 2
	mov edi, [cursor_pos]
	mov byte [0xB8000 + edi], ' '
	mov ah, [vga_color]
	mov byte [0xB8001 + edi], ah
	jmp .done

.handle_enter:
	cmp byte [in_editor], 1
	je .editor_enter

	;buffer = echo hello[enter]
	;goal = echo hello0
	;current = buffer not resetting
	;solved, wasnt resetting buffer_pos lol


	mov edi, [buffer_pos] ;end of buffer
	mov [input_buffer + edi], byte 0 ; null terminate buffer
	
	cmp dword [buffer_pos], 0 ;empty new line if no command and press enter
	je .empty_line

	call .newline

	call parse_command

	cmp byte [skip_newline], 1 ;need this to avoid printing 2 new lines
	je .skip_nl

	call .newline

	mov dword [buffer_pos], 0 ;resetting buffer
	
	cmp byte [in_editor], 1 ;ie in editor so dont want to print shell_prompt
	je .done

	push shell_prompt
	call print
	add esp, 4
	jmp .done

.editor_enter:
	call .newline
	jmp .done

.skip_nl:
	mov byte [skip_newline], 0 ; resetting flag
	mov dword [buffer_pos] , 0 ; resetting buffer 
	push shell_prompt ;printing prompt
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
	;formula = ((cursorpos/160) + 1 ) * 160
	mov eax, [cursor_pos] ;setting up cursor pos
	xor edx, edx ;zeroing out edx for division
	mov ebx, 160 ;  
	div ebx
	inc eax
	mul ebx
	mov [cursor_pos], eax
	ret

init_screen:
	pushad

	xor ebx, ebx
	mov ecx, 2000
.draw:
	mov byte [0xB8000 + ebx], ' '
	mov byte ah, [vga_color]
	mov byte [0xB8001 + ebx], ah ;black
	add ebx, byte 2
	loop .draw
	popad
	ret

section .bss
	input_buffer resb 80 ;reserve 80 bytes for user inputs (line length)

	;not used currently but might need later if i implement command history
	;command_buffer resb 10 ; 10 character command should be more than enough

section .data
	cursor_pos dd 0
	buffer_pos dd 0
	skip_newline db 0	
	
	vga_color db 0x0F

	;responses
	shell_prompt db "Enter command -> ", 0 ;null terminated string
	shell_prompt_len equ $-shell_prompt
	help_response db "Supported Commands: clear, reboot, echo, calc", 0
	unknown_response db "Unknown Command. Try typing 'help'", 0

	shift_pressed db 0
	ctrl_pressed db 0
	in_editor db 0 ;flag specifically for whether or not to print shell_prompt on enter press
	esc_pressed db 0
	enter_editor_flag db 0
	editor_scancode db 0
	editor_mode db 0 ; 0 = normal , 1 insert, 2 = command ?
	editor_filename times 32 db 0

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
