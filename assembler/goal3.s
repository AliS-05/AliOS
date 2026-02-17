BITS 32
GLOBAL _start

_start:

    ; mov tests
    mov eax, 10
    mov ebx, 20
    mov ecx, eax

    ; push / pop
    push eax
    push ebx
    pop edx
    pop esi

    ; add tests
    add eax, ebx
    add ecx, 5

    ; sub tests
    sub ebx, eax
    sub edx, 3

    ; cmp tests
    cmp eax, ebx
    cmp ecx, 15

    ; conditional jump tests
    je equal_label
    jne notequal_label

    nop

equal_label:
    add eax, 1
    jmp after_compare

notequal_label:
    sub eax, 1

after_compare:

    call my_function
    jmp end_label

my_function:
    mov edi, 123
    ret

end_label:
    nop
    ret

