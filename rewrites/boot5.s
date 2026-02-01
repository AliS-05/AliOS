[bits 16]
[org 0x7C00]

start:
	xor ax, ax
	mov ds, ax
	mov ss, ax
	mov es ,ax
	mov sp, ax
	
	cli

	mov sp, 0x9000

	; flip a20 bit
	in al, 0x92
	or al, 2
	out 0x92, al
	; set up gdtr
	lgdt [gdtr]
	;flip cr0
	mov eax, cr0
	or eax, 1
	mov cr0, eax
	
	; load kernel code
	jmp 0x08:start_protected_mode
gdt_start:
	dq 0x0 ;null

gdt_code:
	dw 0xFFFF ; first 16 bits of limit
	dw 0 ;
	db 0 ;  first 24 bits of base
	db 10011010b ; access
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
dw 0x55AA

[bits 32]
start_protected_mode:
	mov byte [0xb8000], "H"
	mov byte [0xb8001], 0x1F
