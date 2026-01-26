; A tiny bootloader that does nothing but stay alive
loop:
    jmp loop

; Fill the rest of the 510 bytes with zeros
times 510-($-$$) db 0
; The magic boot signature (2 bytes)
dw 0xAA55


;REFERENCE
;0x7c00 our address, dont overwrite yourself !!
;safe memory 0x0500 - 0x7BFF about 30,000 bytes (30kb)
; safe memory 2 0x7E00 - 0x9FBFF ~ 622kb

;important memory addresses 
;0x000 - 0x03FF Interrupt vector table (1024 bytes)
;0x0400 - 0x04FF Bios Data (255 bytes)
;0x7C00 - 0x7DFF 511 bytes (Master Boot Record)
;0xA0000 - 0xFFFFF VGA Memory, BIOS ROM, Video RAM (393kb)

; on stack pointers
; basically the stack can go anywhere where we have free memory
; noteably as I have just defined above
;
