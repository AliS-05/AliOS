[org 0x7c00]
bits 16
start:
	xor ax, ax
	mov ds, ax ; dont really understand the register specifics
	mov ss, ax
	mov es, ax
	mov sp, 0x7c00 ;set up stack

	;need to load kernel into memory from disk something like this
	mov ah, 0x02
	mov al, 1
	mov ch, 0
	mov dh, 0 ;needed numbers
	mov cl, 2

	xor bx, bx
	mov es, bx
	mov bx, 0x8000 ; didnt use es just had bx with 8000
	
	int 0x13
	cli

	lgdt [gdtr] ;loading gdt. partially understand

	in al, 0x92 ; flipping a20 bit
	or al, 2 ;was using al changed to eax
	out 0x92, al

	mov eax, cr0 ;forgot
	or eax, 1b
	mov cr0, eax

	jmp 0x08:kernel_start ;copied

times 510-($-$$) db 0
dw 0xAA55


gdt_start:
	dq 0x0; null descriptor

gdt_code:
	dw 0xFFFF ;limit
	dw 0x0000 ;base low
	db 0x00 ;base mid
	db 10011011b ;access byte code ,readable, priviledge 0
	db 11001111b ; granularity 4KB blocks 32 bit mode
	db 0x00 ; base high

gdt_data:
	dw 0xFFFF
	dw 0x00
	db 0x00
	db 10010011b
	db 11001111b
	db 0x00
gdt_end:

gdtr:
	dw gdt_end - gdt_start - 1 ; gdt size
	dd gdt_start;gdt address
	
bits 32
kernel_start:
	mov ax, 0x10 ; was previously zeroing out all registers i guess we need them to be 0x10 ?
	mov ds, ax
	mov ss, ax
	mov es, ax
	mov ebp, 0x90000 
	mov esp, ebp ; forgot this
	
	mov byte [0xB8000], "H"
	mov byte [0xB8001], 0x1F

	mov byte [0xB8002], "I"
	mov byte [0xB8003], 0x1F

	jmp $


