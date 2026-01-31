bits 32
[org 0x8000]

kernel:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov gs, ax
    mov esp, 0x90000

    ; Remap PIC
    mov al, 0x11
    out 0x20, al
    out 0xA0, al

    mov al, 0x20 
    out 0x21, al
    mov al, 0x28 
    out 0xA1, al
    
    mov al, 0x04
    out 0x21, al
    mov al, 0x02
    out 0xA1, al

    mov al, 0x01
    out 0x21, al
    out 0xA1, al

    ; Enable Keyboard IRQ only
    mov al, 0xFD 
    out 0x21, al 
    mov al, 0xFF 
    out 0xA1, al
    
    lidt [idtr]
    sti

    ; Print 'S' to show we entered the kernel
    mov byte [0xB8000], 'S'
    mov byte [0xB8001], 0x0F

    jmp $

idt_start:
    times 32 dq 0 ; Exceptions
    ; int 0x20 timer
    dw timer_handler, 0x08
    db 0, 10001110b
    dw 0
    ; int 0x21 keyboard
    dw keyboard_handler, 0x08
    db 0, 10001110b
    dw 0
    times (256-34) dq 0
idt_end:

idtr:
    dw idt_end - idt_start - 1
    dd idt_start

timer_handler:
    pushad
    mov al, 0x20
    out 0x20, al
    popad
    iretd

keyboard_handler:
    pushad
    mov ax, 0x10
    mov ds, ax

    in al, 0x60
    test al, 0x80
    jnz .done

    movzx ebx, al
    mov al, [scancode_table + ebx]

    mov [0xB8000], al
    mov byte [0xB8001], 0x1F

.done:
    mov al, 0x20
    out 0x20, al
    popad
    iretd

scancode_table:
    db 0, 0, '1' , '2' , '3' , '4' , '5' , '6' , '7' , '8' , '9' , '0' , '-' , '=' , 0 
    db 0, 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', 0  
    db 0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', "'", '`', 0 
    db 0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0 

; Pad kernel to exactly 4KB
times 4096-($-$$) db 0
