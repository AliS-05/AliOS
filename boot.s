[org 0x7c00]; 0x7c00 is location of kernel

start:
	xor ax, ax ;set ax to 0
	mov ds, ax ; point DS to 0 where IVT lives
	
	;Interrupt 9 is at address 0x24 (9*4 bytes)
	mov word [0x24], _keyboard_handler ;storing our label's address
	mov word [0x26], 0 ; storing our segment

	mov esp, 0x7e00 ; setting up stack
	;TODO: remap PIC and Load IDT
	sti ;enables interrupts
loop:
    jmp loop

_keyboard_handler:
	pusha ;save all registers
	;read the scancode from keyboard controller
	in al, 0x60 

	;base address into register
	mov ax, 0xB800
	mov es, ax ; increases register to 0xB8000
	mov byte [es:0], "A"
	mov byte [es:2], 0x0F ;white text on black background

 	;sending end of interrupt to keyboard PIC
	mov al, 0x20 
	out 0x20, al
	
	popa ; reverse of pushad to restore register states
	iretd ; interrupt return


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
