bits 32
[org 0x8000]
kernel:
	cli
	call remap_pic
	call init_screen
	sti
	
	;prints command prompt
	mov esi, shell_prompt
	movzx edi, word [cursor_pos]
	call print_string

	;need to store user input in buffer
	

	jmp $

remap_pic:
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov ss, ax
	mov fs, ax
	mov gs, ax
	mov esp, 0x90000

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
	mov ax, 0x10
	mov ds, ax
	movzx edi, word [cursor_pos] ;saving cursor pos in register

	in al, 0x60 ; reading scancode
	test al, 0x80 ; if scancode is a relase we skip
	jnz .done

	cmp al, 0x0E ;backspace
	je .handle_backspace

	cmp al, 0x1C ;enter
	je .handle_enter
	
	cmp edi, 160
	je .done

	movzx ebx, al ;padding scancode in ebx register
	mov al, [scancode_table + ebx] ; finding actual character

	;NOTE also disables space... need to work around this.
	;test al, al ; checks if printable character ie not a character defined as 0 in the scancode table
	;jz .done

	mov byte [0xB8000 + edi], al
	mov byte [0xB8001 + edi], 0x0F 

	add word [cursor_pos], 2 ;moving to next character

	movzx edi, word [buffer_pos] ; writing to buffer
	inc byte [buffer_pos]
	mov [input_buffer + edi], al ;COMMAND BUFFER
.done:
    mov al, 0x20 ;telling pic we received the message
    out 0x20, al
    popad
    iretd

.handle_backspace:
	cmp edi, 0
	je .done ;at 0,0 theres nowhere to go

	sub word [cursor_pos], 2 ;because we're always infront of the previous character
	movzx edi, word [cursor_pos]
	mov byte [0xB8000 + edi], ' '
	mov byte [0xB8001 + edi], 0x00
	
	;adjust buffer
	dec byte [buffer_pos] ;
	movzx edi, word [buffer_pos]
	mov [input_buffer + edi], byte 0 ;replacing with 0
	jmp .done

.handle_enter:
	movzx edi, word [buffer_pos] ;end of buffer
	mov [input_buffer + edi], byte 0 ; null terminate buffer
	
	cmp word [buffer_pos], 0 ;empty new line if no command and press enter
	je .empty_line

	call .newline

	call .parse_command

	cmp byte [skip_newline], 1
	je .skip_nl

	call .newline

	mov word [buffer_pos], 0 ;resetting buffer
	mov esi, shell_prompt
	movzx edi, word [cursor_pos]
	call print_string
	jmp .done

.skip_nl:
	mov byte [skip_newline], 0
	mov word [buffer_pos] , 0
	mov esi, shell_prompt
	movzx edi, word [cursor_pos]
	call print_string
	jmp .done

.empty_line:
	call .newline
	mov esi, shell_prompt
	movzx edi, word [cursor_pos]
	call print_string
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

.parse_command:
	pushad
	;since its less than 20 commands we can just search every single one without too big a hit of performance

	;checking help command
	mov esi, input_buffer ;string 1
	mov edi, cmd_help ; string 2
	call strcmp
	je .help_cmd
	
	;checking clear command
	mov esi, input_buffer
	mov edi, cmd_clear
	call strcmp
	je .clear_cmd

	;checking reboot command
	mov esi, input_buffer
	mov edi, cmd_reboot
	call strcmp
	je .reboot_cmd

	jmp .unknown_cmd

.help_cmd:
	mov esi, help_response
	movzx edi, word [cursor_pos]
	call print_string
	jmp .enter_done
.clear_cmd:
	call init_screen
	xor ax, ax
	mov [cursor_pos], ax
	mov byte [skip_newline], 1
	;mov esi, shell_prompt
	;movzx edi, word [cursor_pos]
	;call print_string
	jmp .enter_done
.reboot_cmd:
	jmp 0xFFFF:0 ;osdevwiki says this is the easiest way and it works so ...
.unknown_cmd:
	mov esi, unknown_response
	movzx edi, word [cursor_pos]
	call print_string

.enter_done:
	popad
	ret

;.empty_new_line:
;	mov esi, shell_prompt
;	movzx edi, word [cursor_pos]
;	call print_string
;	jmp .enter_done

strcmp:
	mov al, [esi]
	mov bl, [edi]
	cmp al, bl
	jne .not_equal
	test al, al ;null terminator ?
	jz .equal
	inc esi
	inc edi
	jmp strcmp
.equal:
	cmp al, al ;sets 0 flag so we know whatever command was most recently check is a match
	ret

.not_equal:
	;print error message
	or al, 1; this command was not a match clear zero flag
	ret

scancode_table: ; NOTE need to fix this is there some replacement for the zeros ? its just printing blank spaces
	db 0, 27, '1' , '2' , '3' , '4' , '5' , '6' , '7' , '8' , '9' , '0' , '-' , '='
	db 0x08, 0x09, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']'
	db 13, 0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', "'", '`'
	db 0, '\', 'z','x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0
	db 0, 0, 0, ' ' ; 0x39 = spacebar

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

print_string:
	cld
	; assumes the caller has string stored in esi , cursor location in edi
	push eax
	push esi
	.loop:
		lodsb ; loads char into al
		test al, al ;  checks if current character is 0 ie string terminator
		jz .done
		mov byte [0xB8000 + edi], al
		mov byte [0xB8001 + edi], 0x0F
		add edi, 2
		jmp .loop
.done:
	mov ax, di
	mov [cursor_pos], ax
	pop esi
	pop eax
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
	help_response db "Supported Commands: clear, reboot", 0
	unknown_response db "Unknown Command. Try typing 'help'", 0
; Pad kernel to exactly 4KB
times 4096-($-$$) db 0
