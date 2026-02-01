bits 32
[org 0x8000]
kernel:
	cli
	call remap_pic
	call init_screen
	sti
	
	mov esi, shell_prompt
	movzx edi, word [cursor_pos]
	call print_string

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
    

    movzx ebx, al ;padding scancode in ebx register
    mov al, [scancode_table + ebx] ; finding actual character
    
    ;NOTE also disables space... need to work around this.
    ;test al, al ; checks if printable character ie not a character defined as 0 in the scancode table
    ;jz .done
    
    mov byte [0xB8000 + edi], al
    mov byte [0xB8001 + edi], 0x0F 

    add word [cursor_pos], 2 ;moving to next section

.done:
    mov al, 0x20 ;telling pic we received the message
    out 0x20, al
    popad
    iretd

.handle_backspace:
	cmp edi, 0
	je .done ;at 0,0 theres nowhere to go

	sub word [cursor_pos], 2 ;because we're always infront of the previous character
	mov edi, [cursor_pos]
	mov byte [0xB8000 + edi], ' '
	mov byte [0xB8001 + edi], 0x0F
	
	jmp .done

.handle_enter:
	; next line = (cursor_pos / 160) + 1 * (160)
	xor dx ,dx ; need to zero out bc stores remainder
	mov ax, [cursor_pos]
	mov bx, 160
	div bx
	add ax, 1
	mul bx

	
	mov word [cursor_pos], ax ;new location stored in ax
	mov esi, shell_prompt
	movzx edi, word [cursor_pos]
	call print_string

	;mov edi, [cursor_pos] 
	;mov word [0xB8000 + edi], 0x1F00
	jmp .done

scancode_table: ; NOTE need to fix this is there some replacement for the zeros ? its just printing blank spaces
	db 0, 0, '1' , '2' , '3' , '4' , '5' , '6' , '7' , '8' , '9' , '0' , '-' , '=' , 0 
	db 0, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\\'  
	db '\\', 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', "'", '`', 0 
	db 0, 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, 0         ; 0x2A-0x35	
	db 0, ' ' ; 0x39 = spacebar

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
	mov [cursor_pos], di
	pop esi
	pop eax
	ret

section .data
	cursor_pos dw 0
	shell_prompt db "Enter command -> ", 0 ;null terminated string
; Pad kernel to exactly 4KB
times 4096-($-$$) db 0
