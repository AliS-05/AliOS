bits 16
[org 0x7c00]
start:
	;set up segment registers + stack
	xor ax, ax
	mov ds, ax
	mov ss, ax
	mov es, ax

	mov sp, 0x9000 ;grows downward so this gives us about 5kb of stack more than enough for bootsector

	mov ah, 0x02
	mov al, 1
	mov ch, 0
	mov dh, 0
	mov cl, 2
	xor bx, bx
	mov es, bx
	mov bx, 0x7E00
	int 0x13

	cli ; oops forgot this , from osdevwiki
	; flip a20 bit
	in al, 0x92 
	or al, 2 ;i dont actually remember why we do this operation exactly
	out 0x92, al
	 ; set up gdt
	lgdt [gdtr]
	;flip cr0 bit
	mov eax, cr0
	or eax , 1; flips last bit 
	mov cr0, eax

	jmp 0x08:kernel ;jumps into kernel code segment 

gdt_start:
	dq 0 ;null desc
gdt_code:
	dw 0xFFFF ;max limit
	dw 0 ;base low
	db 0 ;base mid
	db 10011010b;access byte direction bit needs to be 0 ?
	db 11001111b ;flags + limit byte
	db 0;base low
gdt_data:
	dw 0xFFFF ;max limit
	dw 0 ;base low
	db 0 ;base mid
	db 10010010b;this doesnt look right
	db 11001111b ;flags + limit byte
	db 0;base low
gdt_end:
gdtr:
	dw gdt_end - gdt_start - 1 ;length
	dd gdt_start;this needs to be the actual pointer to the gdt so $ ?
times 510-($-$$) db 0
dw 0xAA55;little endian

bits 32
kernel:
	mov ax, 0x10
	mov ds, ax
	mov es, ax
	mov ss, ax
	mov fs, ax
	mov gs, ax
	mov esp, 0x90000

	%macro io_delay 0
	    jmp short $+2
	    jmp short $+2
	%endmacro

	mov al, 0x11
	; remap pic
	out 0x20, al ; initialize command
	out 0xA0, al

	mov al, 0x20 
	out 0x21, al;gdt code
	mov al, 0x28 
	out 0xA1, al;gdt data idk
	
	mov al, 0x04
	out 0x21, al
	mov al, 0x02
	out 0xA1, al

	mov al, 0x01
	out 0x21, al
	out 0xA1, al

	mov al, 0xFD ;disables all IRQ's except keyboard 
	out 0x21, al ; bit 1 = 0 = IRQ1 enables
	mov al, 0xFF ; ; bit 0 = 1 = IRQ0 (timer) disabled
	out 0xA1, al
	
	lidt [idtr]
	sti
	;set up idt
	mov byte [0xB8000], 'S'
	mov byte [0xB8001], 0x0F
	;write to screen
	jmp $

idt_start:
	times 32 dq 0 ; ignore everything until keyboard interrupt
	; int 0x20 timer
	dw timer_handler
	dw 0x08
	db 0
	db 10001110b
	dw 0	
	; int 0x21 ; keyboard
	dw keyboard_handler
	dw 0x08
	db 0
	db 10001110b
	dw 0

	times (256-34) dq 0 ; zeroing out the rest
idt_end:
idtr:
	dw idt_end - idt_start - 1 ; length
	dd idt_start

timer_handler:
	pushad
	mov al, 0x20
	out 0x20, al
	popad
	iretd

keyboard_handler:

	pushad ;d = 32 bits right ?
	mov ax, 0x10
	mov ds, ax
	;read scancode
	in al, 0x60
	test al, 0x80 ; key releases
	jnz .done

	movzx ebx, al
	mov al, [scancode_table + ebx]

	mov byte [0xB8000], al
	mov byte [0xB8001], 0x1F

.done:
	mov al, 0x20
	out 0x20, al
	popad
	iretd

scancode_table: ;allows for O(1) lookups on every character
	db 0, 0, '1' , '2' , '3' , '4' , '5' , '6' , '7' , '8' , '9' , '0' , '-' , '=' , 0 ; 0x00-0x0E
	db 0, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 0    ; 0x0F-0x1C
	db 0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', "'", '`', 0    ; 0x1D-0x29
	db 0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0         ; 0x2A-0x35	

times 4096-($-$$) db 0
