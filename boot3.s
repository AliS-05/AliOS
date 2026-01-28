bits 16
[org 0x7C00]


start:
	xor ax, ax
	mov bx, ax
	mov es, ax
	mov ss, ax
	mov sp, 0x7C00 ;setting up stack pointer

	lgdt [gdtr]


	in al, 0x92 ;reads current state of System Control Port
	or al, 2 ;sets the second bit to high
	out 0x92, al ;writes it back effectively flipping the A20 pin

	mov eax, CR0
	OR eax, 1b ;flipping CR0 bit for protected mode
	mov CR0, eax
	
	jmp 0x08:init_pm

gdt_start:
	dq 0x0; null descriptor

gdt_code:
	dw 0xFFFF ;limit
	dw 0x0000 ;base low
	db 0x00 ;base mid
	db 10011010b ;access byte code ,readable, priviledge 0
	db 11001111b ; granularity 4KB blocks 32 bit mode
	db 0x00 ; base high

gdt_data:
	dw 0xFFFF
	dw 0x00
	db 0x00
	db 10010010b
	db 11001111b
	db 0x00
gdt_end:

gdtr:
	dw gdt_end - gdt_start - 1 ; gdt size
	dd gdt_start;gdt address
	

[bits 32]
init_pm:
	mov ax, 0x10 ; update segment registers with data selector
	mov ds, ax
	mov es, ax
	mov ss, ax
	mov fs, ax
	mov gs, ax
	mov ebp, 0x90000
	mov esp, ebp

	mov byte [0xB8000], "H"
	mov byte [0xB8001], 0x4F

	mov byte [0xB8002], "i"
	mov byte [0xB8003], 0x4F
	cli

spin:
	jmp spin

msg db "Hello From Protected Mode!", 0

TIMES 510-($-$$) db 0 ;zeroing out MBR
dw 0xAA55 ; MBR signature
