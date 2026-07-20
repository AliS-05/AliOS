bits 32

extern bss_start
extern bss_end
extern kernel_main
extern parse_command
extern print
extern edit_loop
extern editor_char
extern scrollUp
extern scrollDown
extern newLine
extern putChar
extern prevCommandHistory
extern nextCommandHistory
extern moveLeft
extern moveRight
extern nic_irq_handle

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
global editor_extension
global program_running

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

	    push dword editor_extension 
	    push dword editor_filename
	    call edit_loop
	    add esp, 8
	    ; restore shell after editor exits
	    push shell_prompt
	    call print
	    add esp, 4
	    jmp .idle

remap_pic:
	; Remap PIC
	mov al, 0x11 ;initialization
	out 0x20, al ;pic1_control reg
	out 0xA0, al ;pic2_control

	mov al, 0x20 ;IRQs 0-7
	out 0x21, al ;PIC1_data
	mov al, 0x28 ;IRQs 8-15
	out 0xA1, al ;pic2_data

	mov al, 0x04 ;setting IR line 2 connecting pics
	out 0x21, al
	mov al, 0x02
	out 0xA1, al

	mov al, 0x01 ;bit 0  enables 8086 mode
	out 0x21, al
	out 0xA1, al
	
	mov al, 0
	out 0x21, al
	out 0xA1, al
	; Enable Keyboard IRQ only
	;mov al, 0xFD 
	;out 0x21, al 
	;mov al, 0xFF 
	;out 0xA1, al

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
	;int 0x2B nic slave irq
	times 9 dq 0
	dw nic_slave_irq, 0x08
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

nic_slave_irq:
	pushad
	call nic_irq_handle
	popad
	
	mov al, 0x20
	out 0xA0, al
	out 0x20, al
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
	
	cmp al, 0xE0
	je .extPrefix

	test al, 0x80 
	jnz .check_release

	cmp al, 0x01 ;escape key
	jne .normal

	mov byte [esc_pressed] , 1
	jmp .done

.extPrefix:
	mov byte [ext_prefix], 1
	jmp .done

;editor label
.normal:
	mov cl, [ext_prefix]
	mov byte [ext_prefix], 0
	cld
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

	; ctrl + keypress
	cmp byte [ctrl_pressed], 1
	je .ctrlHandle
	cmp cl, 1
	je .done

	jmp .got_char

.ctrlHandle:
	cmp byte cl, 1
	je .arrowKeys
		;ctrl + 'u' scroll up
	cmp dword ebx, 0x16
	je .scroll_up
	;ctrl + 'd' scroll down
	cmp dword ebx, 0x20
	je .scroll_down
	jmp .done
.arrowKeys:
	;ctrl + 'upArrow' last command
	cmp dword ebx, 0x48
	je .prevCommand
	;ctrl + 'downArrow' next command
	cmp dword ebx, 0x50
	je .nextCommand
	
	;ctrl + 'leftArrow' moves cursor left
	cmp dword ebx, 0x4B
	call moveLeft
	;ctrl + 'rightArrow' moves cursor right
	cmp dword ebx, 0x4D
	call moveRight

	jmp .done

.prevCommand:
	call prevCommandHistory
	jmp .done

.nextCommand:
	call nextCommandHistory
	jmp .done

.scroll_up:
	call scrollUp
	jmp .done

.scroll_down:
	call scrollDown
	jmp .done

.use_shifted:
	mov byte al, [scancode_table_shifted + ebx]

.got_char:
	mov byte [editor_char], al
	cmp byte [in_editor], 1
	jne .shell_echo

	;editor manages its own screen 
	mov byte [0xB8000 + edi], al
	mov byte ah, [vga_color]
	mov byte [0xB8001 + edi], ah
	add dword [cursor_pos], 2
	jmp .done

.shell_echo:
	;snake program
	cmp byte [program_running], 1
	je .done

	; echo through C so it scrolls + lands in the scroll buffer
	movzx eax, al
	push eax
	call putChar
	add esp, 4

	mov al, [editor_char]
	mov edi, [buffer_pos]
	cmp edi, 79
	jae .done
	mov [input_buffer + edi], al
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

;mostly the same logic for this section
.shift_press:
	mov byte [shift_pressed], 1
	mov byte [editor_char], 0
	jmp .done

.shift_release:
	mov byte [shift_pressed], 0
	mov byte [editor_char], 0
	jmp .done

.ctrl_press:
	mov byte [ctrl_pressed], 1
	mov byte [editor_char], 0
	jmp .done

.ctrl_release:
	mov byte [ctrl_pressed], 0
	mov byte [editor_char], 0
	jmp .done


.done:
    mov al, 0x20 ;telling pic we received the message
    out 0x20, al
    popad
    iretd

.handle_backspace:

	cmp byte [in_editor], 1 ;special case
	je .editor_backspace
	
	cmp dword [buffer_pos], 0 ;nothing to do if at beginning of line / buffer
	je .done

	cmp edi, 0
	je .done ;at 0,0 theres nowhere to go

	;adjust buffer
	dec dword [buffer_pos] ;going back in buffer

	push edi
	mov edi, [buffer_pos] ;adding zero
	mov byte [input_buffer + edi], 0
	pop edi

	sub dword [cursor_pos], 2 ;moving cursor back one (2 bytes = 1 square)
	mov edi, [cursor_pos];

	mov byte [0xB8000 + edi], ' '
	mov byte ah, [vga_color]
	mov byte [0xB8001 + edi], ah
	
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

	push dword 10           ; '\n'
        call putChar
        add esp, 4

	call parse_command


	cmp byte [skip_newline], 1 ;need this to avoid printing 2 new lterminalScrollPosition < 26 || ines
	je .skip_nl

	push dword 10           ; '\n'
        call putChar
        add esp, 4	

	mov dword [buffer_pos], 0 ;resetting buffer
	
	cmp byte [in_editor], 1 ;ie in editor so dont want to print shell_prompt
	je .done

	push shell_prompt
	call print
	add esp, 4
	jmp .done

.editor_enter:
	push dword 10           ; '\n'
        call putChar
        add esp, 4
	jmp .done

.skip_nl:
	mov byte [skip_newline], 0 ; resetting flag
	mov dword [buffer_pos] , 0 ; resetting buffer 
	push shell_prompt ;printing prompt
	call print
	add esp, 4

	jmp .done

.empty_line:
	push dword 10           ; '\n'
        call putChar
        add esp, 4
	push shell_prompt
	call print
	add esp, 4
	
	jmp .done

.newline:

	push dword [cursor_pos]
	call newLine
	mov [cursor_pos], eax
	add esp, 4

	;C code seems to be working keeping assembly here just in case

	;;formula = ((cursorpos/160) + 1 ) * 160
	;mov eax, [cursor_pos] ;setting up cursor pos
	;xor edx, edx ;zeroing out edx for division
	;mov ebx, 160 ;  
	;div ebx
	;inc eax
	;mul ebx
	;mov [cursor_pos], eax
	ret

init_screen:
	pushad

	xor ebx, ebx
	mov ecx, 2000
.draw:
	mov byte [0xB8000 + ebx], ' '
	mov byte ah, [vga_color]
	mov byte [0xB8001 + ebx], ah ;black by default 
	add ebx, byte 2
	loop .draw
	popad
	ret

section .bss
	input_buffer resb 80 ;reserve 80 bytes for user inputs (line length)
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
	ext_prefix db 0
	in_editor db 0 ;flag specifically for whether or not to print shell_prompt on enter press
	program_running db 0 ;program not running by default
	esc_pressed db 0
	enter_editor_flag db 0
	editor_scancode db 0
	editor_mode db 0 ; 0 = normal , 1 insert, 2 = command ?
	editor_filename times 8 db 0
	editor_extension times 3 db 0
	;ps/2 Set 1 Make in PS/2 scancode chart
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
