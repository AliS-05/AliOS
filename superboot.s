bits 16
[org 0x7c00]

start:
	;
	xor ax, ax
	mov ds, ax
	mov ss, ax
	mov es, ax
	mov sp, 0x9000  ;stack pointer
	
	; loading kernel from 'disk'
	mov ah, 0x02    
	mov al, 100
	mov ch, 0       
	mov dh, 0       
	mov cl, 2       
	mov bx, 0x8000  
	int 0x13 ;bios call

	; Enter Protected Mode
	cli
	 
	in al, 0x92  ;flipping a20 line
	or al, 2
	out 0x92, al

	lgdt [gdtr]
	mov eax, cr0 ;flipping cr0 bit 
	or eax, 1
	mov cr0, eax

	jmp 0x08:0x8000 ; jumping to kernel

gdt_start:
    dq 0 
gdt_code:
    dw 0xFFFF, 0
    db 0, 10011010b, 11001111b, 0
gdt_data:
    dw 0xFFFF, 0
    db 0, 10010010b, 11001111b, 0
gdt_end:
gdtr:
    dw gdt_end - gdt_start - 1
    dd gdt_start

times 510-($-$$) db 0
dw 0xAA55
