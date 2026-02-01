bits 16

[org 0x7c00] ;kernel location

start:
	xor ax, ax ;ax = 0
	mov ds, ax ; ds = 0 for offsets
	mov es, ax ; for string operations
	mov ss, ax ; stack segment = 0

	mov sp, 0x7e00 ; - 0x9FBFF is safe memory initialize stack pointer

	call _init_screen
	mov word [cursor_pos], 0

	cli ;disables interrupts
	mov word [0x24], _keyboard_handler ; initalize IVT not IDT since in real mode
	mov word [0x26], ax
	sti ;enables interrupts

_loop:
	hlt ;infinite loop do nothing
	jmp _loop

_init_screen:
	;mov ch, 0x24 ;rows
	;mov cl, 0x49 ;columns
	
	mov bx, 0xB800
	mov es, bx
	mov di, [cursor_pos]
	
.loop:
	mov word [es:di], 0x1F00 ;blank blue space
	add di, 2
	cmp di, 4000 ; if not last slot
	jne .loop

	ret


_keyboard_handler:
	pusha
	push es
	mov di, [cursor_pos] ;cursor offset

	in al, 0x60 ;read scancode from port 60
	cmp al, 0x80 ;this is just checking if the key is a make or break, if its a break key we let the pic know were done
	jae .done ;jump if above or equal, ie if the condition is met

	cmp al, 0x0E
	je .handle_backspace ;checking for backspace

	cmp al, 0x1C
	je .handle_enter ;checks for enter

	mov bx, 0xB800
	mov es, bx ; memory address offset for VGA

	movzx bx, al ;moves scancode into base register
	mov al, [scancode_table + bx] ;reading ASCII char
	
	cmp al, 0
	je .done ;dont print empty characters

	mov byte [es:di], al
	mov byte [es:di+1], 0x1F
	add di, 2
	mov [cursor_pos], di ; next char in 2 bytes
	jmp .done

;tab 0x0F (0x8F)
; caps 0x3A (0xBA)

.handle_enter:
	mov bx, 0xB800
	mov es, bx
	mov bx, 160
	xor ax, ax
	xor dx, dx
	;push ax ; ax is required for div ax is currently storing keypressed but obv we already know its enter so we can discard

	;formula for new line is ((cur_line / 160) + 1) * (160)
	mov ax, [cursor_pos] ; ie [cursor_pos]
	div bx
	add ax, 1
	mul bx

	mov [cursor_pos], ax ; beginning of new line
	mov di, [cursor_pos]
	mov word [es:di], 0x1F00 ; empty blue square
	jmp .done ; all done !
	
.handle_backspace:
	cmp di, 0 ;doesnt go off screen (wont go up a line either)
	je .done
	mov bx, 0xB800
	mov es, bx ;correct memory segment
	sub di, 2 ; subtract from offset
	mov byte [es:di], 0x00 ;clears char and color
	mov byte [es:di+1], 0x1F
	mov [cursor_pos], di

.done:
	mov al, 0x20
	out 0x20, al ;tell keyboard PIC we have recieved the keypress
	
	pop es
	popa
	iret

section.data:
	cursor_pos dw 0

	scancode_table: ;allows for O(1) lookups on every character
		db 0, 0, '1' , '2' , '3' , '4' , '5' , '6' , '7' , '8' , '9' , '0' , '-' , '=' , 0 ; 0x00-0x0E
		db 0, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\'   ; 0x0F-0x1C
		db 0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', "'", '`', 0    ; 0x1D-0x29
		db 0, 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, 0         ; 0x2A-0x35	
		db 0, ' ' ; 0x39 = spacebar

times 510-($-$$) db 0 ; filling MBR with 0's
dw 0xAA55 ;MBR signature
