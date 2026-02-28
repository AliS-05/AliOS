org 0x200000
bits 32
global start
center equ 2000

section .data
	xPos dd 40
	yPos dd 12

start:
	;pushad
	cld
	call init_screen
	call draw_border

.loop:
	call read_input
	call draw_player
	call delay
	jmp .loop
	;popad
	;mov eax, 0	; return code
	;ret	; necessary

read_input:
	in al, 0x64 ;reading keyboard status
	test al, 1 ;no key pressed
	jz .done
	
	in al, 0x60

	test al, 0x80 ;ignoring key releases
	jnz .done
	
	cmp al, 0x11 ; W
	je .up

	cmp al, 0x1F ; S
	je .down

	cmp al, 0x1E ;A
	je .left

	cmp al, 0x20 ;D
	je .right

	jmp .done

.up:
	dec dword [yPos]
	jmp .done

.down:
	inc dword [yPos]
	jmp .done

.left:
	dec dword [xPos]
	jmp .done

.right:
	inc dword [xPos]
	jmp .done

.done:
	ret	

delay:
	mov ecx, 5000000
.loop:
	dec ecx
	jnz .loop
	ret

draw_player:
	call init_screen
	call draw_border

	mov eax, [yPos]
	mov ebx, 160
	mul ebx

	mov ebx, [xPos]
	shl ebx, 1

	add eax, ebx
	add eax, 0xB8000

	mov byte [eax], '@'
	mov byte [eax+1], 0x0A

	ret

init_screen:
	;pushad
	xor ebx, ebx
	mov ecx, 2000
.draw:
	mov byte [0xB8000 + ebx], ' '
	mov byte [0xB8001 + ebx], 0x00
	add ebx, 2
	loop .draw
	;popad
	ret

draw_border:
	mov ecx, 60;60  characters
	mov ebx, 20 ; 10 spaces forward (offset)
.top_border:
	mov byte [0xB8000 + ebx], 0xCD ; top pipe character
	mov byte [0xB8001 + ebx], 0x02 ; black background white foreground
	add ebx, 2
	loop .top_border

	mov ebx, 20 ;left side border position
	mov ecx, 25
.left_border:
	mov byte [0xB8000 + ebx], 0xBA ; straight pipe character
	mov byte [0xB8001 + ebx], 0x02
	add ebx, 160 ;next line
	loop .left_border

	mov ebx, 140 ;right side border position
	mov ecx, 25 ;repetitions
.right_border:
	mov byte [0xB8000 + ebx], 0xBA
	mov byte [0xB8001 + ebx], 0x02
	add ebx, 160
	loop .right_border

	mov ebx, 3860 ; beginning of last line + offset
	mov ecx, 60
.bottom_border:
	mov byte [0xB8000 + ebx], 0xCD
	mov byte [0xB8001 + ebx], 0x02
	add ebx, 2
	loop .bottom_border
	ret
