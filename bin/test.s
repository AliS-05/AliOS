bits 32
section .data
	string db "Hello from bin test", 0

global start

_start:
	mov edi, 0xB8000
	mov word [edi], 0x0F54
	mov word [edi+2], 0x0F45
	mov word [edi+4], 0x0F53
	mov word [edi+6], 0x0F54
	ret
