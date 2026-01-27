bits 16
[org 0x7c00] ;kernel location

start:
	xor ax, ax ;ax = 0
	mov ds, ax ; ds = 0 for offsets
	mov es, ax ; for string operations
	mov ss, ax ; stack segment = 0

	mov sp, 0x7e00 ; - 0x9FBFF is safe memory initialize stack pointer
	
	cli
	mov word [0x24], _keyboard_handler ; initalize IVT not IDT since in real mode
	mov word [0x26], ax
	sti ;enables interrupts

_loop:
	hlt ;infinite loop do nothing
	jmp _loop

_keyboard_handler:
	pusha
	push es

	in al, 0x60 ;read scancode from port 60
	cmp al, 0x80
	jae .done
	mov bx, 0xB800

	mov es, bx
	
	movzx bx, al ;moves scancode into base register
	mov al, [scancode_table + bx] ;reading ASCII char

	mov byte [es:0], al
	mov byte [es:1], 0x1F

.done:
	mov al, 0x20
	out 0x20, al ;tell keyboard PIC we have recieved the keypress

	pop es
	popa
	iret

scancode_table: ;allows for O(1) lookups on every character
	db 0, 0, '1' , '2' , '3' , '4' , '5' , '6' , '7' , '8' , '9' , '0' , '-' , '=' , 0 ; 0x00-0x0E
	db 0, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 0    ; 0x0F-0x1C
	db 0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', "'", '`', 0    ; 0x1D-0x29
	db 0, '\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0         ; 0x2A-0x35	


times 510-($-$$) db 0 ; filling MBR with 0's
dw 0xAA55 ;MBR signature
