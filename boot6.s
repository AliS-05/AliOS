bits 16
[org 0x7C00]
start:
	xor ax, ax
	mov ds, ax
	mov ss ,ax
	mov es, ax

	mov sp, 0x7c00

	mov ah, 0x02
	mov al, 1
	mov ch, 0
	mov dh, 0
	mov cl, 2
	xor bx, bx
	mov es, bx
	mov bx, 0x8000
	int 0x13

	in al, 0x92
	or al, 2
	out 0x92, al ;flips a20 line
	lgdt [gdtr]

	cli
	
	mov eax , cr0
	or eax, 1b
	mov cr0, eax ;enable 32 bit mode

	jmp 0x08:kernel

gdt_start:
	dq 0x0 ;null desc
gdt_code:
	dw 0xFFFF ;limit
	dw 0;base
	db 0 ;24th bit base
	db 10011010b
	db 11001111b
	db 0
gdt_data:
	dw 0xFFFF
	dw 0
	db 0
	db 10010010b
	db 11001111b
	db 0
gdt_end:
gdtr:
	dw gdt_end - gdt_start - 1
	dd gdt_start

times 510-($-$$) db 0
dw 0xAA55

[bits 32]
[org 0x8000]
kernel:
	
	mov byte [0xB8000], "H"
	mov byte [0xB8001], 0x1F

	mov byte [0xB8002], "I"
	mov byte [0xB8003], 0x1F

	mov al, 0x11
	out 0x20, al ;start init pic remap
	out 0xA0, al

	mov al, 11111101b
	out 0x21, al
	mov al, 11111111b
	out 0xA1, al

	mov al, 0x04
	out 0x21, al
	mov al,0x02
	out 0xA1, al

	mov al, 0x01
	out 0x21, al
	out 0xA1, al

	xor al, al
	out 0x21, al
	out 0xA1, al

	lidt [idtr]
	sti
	jmp $

idt_start:
	times 33 dq 0 ;skipping to entry 33 (0x21)
	dw keyboard_handler; low 16 bits
	dw 0x08 ; gdt_code so has kernel level + can be executed
	db 0
	db 10001110b ; 32 bit interrupt gate
	dw 0 ; high 16 bits
	times (256-34) dq 0; fills rest with null
idt_end:
idtr:
	dw idt_end - idt_start - 1
	dd idt_start


keyboard_handler:
	pusha ;save all registers
	;read the scancode from keyboard controller ( PIC stores scancode in port 60)
	in al, 0x60 

	test al, 0x80 ;scancodes over 0x80 are key releases so this checks for key presses only
	jnz .done ;jumps to done

	movzx ebx, al ;moves scancode into bx for padded indexing
	mov al, [scancode_table + ebx] ;accessing ascii char
	
	mov byte [0xB8000], al
	mov byte [0xB8001], 0x1F ;white text on blue background

.done:
	mov al, 0x20 ;end of interrupt
	out 0x20, al
	popa
	iret

scancode_table: ;allows for O(1) lookups on every character
	db 0, 0, '1' , '2' , '3' , '4' , '5' , '6' , '7' , '8' , '9' , '0' , '-' , '=' , 0 ; 0x00-0x0E
	db 0, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 0    ; 0x0F-0x1C
	db 0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', "'", '`', 0    ; 0x1D-0x29
	db 0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0         ; 0x2A-0x35	




 ; 1024 bytes for now but can be increased
