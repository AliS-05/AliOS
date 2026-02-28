org 0x200000
bits 32
global start
center equ 2000
score equ 0
section .data
	xPos dd 40
	yPos dd 12
	curDirection db 3 ; 0 = W ; 1 = A  ; 2 = S  ; 3 = D
start:
	;pushad
	cld
	call init_screen
	call draw_border

.mainLoop:
	call read_input
	call update_position
	call draw_player
	call delay
	jmp .mainLoop
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
	mov byte [curDirection] , 0 ; up
	jmp .done

.down:
	mov byte [curDirection], 2 ; down
	jmp .done

.left:
	mov byte [curDirection], 1 ; left
	jmp .done

.right:
	mov byte [curDirection], 3 ; right
	jmp .done

.done:
	ret	

delay:
	mov ecx, 20000000
.loop:
	dec ecx
	jnz .loop
	ret

update_position:
	mov al, [curDirection]
	cmp al, 0
	je .up

	cmp al, 1
	je .left

	cmp al, 2
	je .down

	cmp al, 3
	je .right

	ret

.up:
	dec dword [yPos]
	ret
.down:
	inc dword [yPos]
	ret
.left:
	dec dword [xPos]
	ret
.right:
	inc dword [xPos]
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
