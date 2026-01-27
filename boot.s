[org 0x7c00]; setting origin to 0x7c00 which is location of kernel 

start:
	xor ax, ax ;set ax to 0
	mov ds, ax ; point data segment to 0 where IVT lives  
	
	mov es, ax ; es = 0
	mov ss, ax ; stack segment = 0
	mov sp, 0x7e00 ; stack grows down from bootloader


	;Interrupt 9 is at address 0x24  ( 24 in hex is 36, intterupt 9 in IVT * 4 bytes per interrupr)
	mov word [0x24], _keyboard_handler ;storing our label's address
	mov word [0x26], 0x0000 ; storing our segment

	;TODO: remap PIC and Load IDT
	sti ;enables interrupts
loop:
	hlt ; halt until intterupt
	jmp loop

_keyboard_handler:
	pusha ;save all registers
	;read the scancode from keyboard controller ( PIC stores scancode in port 60)
	in al, 0x60 

	test al, 0x80 ;scancodes over 0x80 are key releases so this checks for key presses only
	jnz .done ;jumps to done

	movzx bx, al ;moves scancode into bx for padded indexing
	mov al, [scancode_table + bx] ;accessing ascii char
	
	mov bx, 0xB800
	mov es, bx ; increases register to 0xB8000 ( VGA Memory )
	mov byte [es:0], al
	mov byte [es:1], 0x1F ;white text on blue background

.done:
	mov al, 0x20 ;end of interrupt
	out 0x20, al
	popa
	iret

scancode_table: ;allows for O(1) lookups on every character
	db 0, 0, '1' , '2' , '3' , '4' , '5' , '6' , '7' , '8' , '9' , '0' , '-' , '=' , 0 ; 0x00-0x0E
	db 0, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 0    ; 0x0F-0x1C
	db 0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', "'", '`', 0    ; 0x1D-0x29
	db 0, '\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0         ; 0x2A-0x35	


; Fill the rest of the 510 bytes with zeros
times 510-($-$$) db 0

; NOTE little-endianness so this is actually 55aa
dw 0xAA55 ;mandatory MBR signature last 2 bytes


;REFERENCE
;0x7c00 our address, dont overwrite yourself !!
;safe memory 0x0500 - 0x7BFF about 30,000 bytes (30kb)
; safe memory 2 0x7E00 - 0x9FBFF ~ 622kb

;important memory addresses 
;0x000 - 0x03FF Interrupt vector table (1024 bytes)
;0x0400 - 0x04FF Bios Data (255 bytes)
;0x7C00 - 0x7DFF 511 bytes (Master Boot Record)
;0xA0000 - 0xFFFFF VGA Memory, BIOS ROM, Video RAM (393kb)
;0xB8000 - VGA Memory ^^ each character takes 2 bytes, ascii character and color of text and background

; on stack pointers
; basically the stack can go anywhere where we have free memory
; noteably as I have just defined above
;
