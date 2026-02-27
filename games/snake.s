org 0x200000
bits 32

global start

center equ 2000

start:
	mov edi, 0xB8000
	mov byte [edi + center], 0xDB ; full block character
	mov byte [edi + center + 1], 0xFF ; white on white

	mov eax, 0 ;return code
	ret ;necessary 

