org 0x200000
bits 32
global start
center equ 2000

section .data
	body times 200  dd 0
	xPos            dd 40
	yPos            dd 12
	curDirection    db 3      ; 0 = W ; 1 = A  ; 2 = S  ; 3 = D
	score           db 0
	prevScore       db 0
	applePos        dd 0
	gameOver        db 0      ; 0 = OK 1 = GAME OVER
	gameOverMessage db "Game Over, Play Again ?", 0
	gameOverInstructions db "Press Enter To Play Again Or ESC To Return To Shell.", 0
	scoreMsg db "Score: ", 0
section .text
	

start:
	pushad
	cld
	call spawn_apple
	call mainLoop
.returnToShell:
	popad
	xor eax, eax
	ret
mainLoop:
	call read_input
	call update_position

	mov  al, [score]
	cmp  al, [prevScore]
	je   .skip_apple 
	; if prevScore and score are the same then score has not been updated 
	;meaning apple has not been collected so no need to draw a new one

	mov  [prevScore], al
.skip_apple:

	call draw_player
	call draw_apple
	call draw_score
	call collisions
	call delay
	jmp  mainLoop

read_input:
	in   al, 0x64                 ;reading keyboard status
	test al, 1                    ;no key pressed
	jz   .done
	
	in   al, 0x60

	test al, 0x80                 ;ignoring key releases
	jnz  .done
	
	cmp  al, 0x11                 ; W
	je   .up

	cmp  al, 0x1F                 ; S
	je   .down

	cmp  al, 0x1E                 ;A
	je   .left

	cmp  al, 0x20                 ;D
	je   .right
	
	cmp  al, 0x1C                 ;return
	je .enter

	cmp al, 0x01		      ; escape
	je .esc

	jmp  .done
.up:	
	mov  byte [curDirection], 0   ; up
	jmp  .done

.down:
	mov  byte [curDirection], 2   ; down
	jmp  .done

.left:
	mov  byte [curDirection], 1   ; left
	jmp  .done

.right:
	mov  byte [curDirection], 3   ; right
	jmp  .done

.enter:
	cmp  byte [gameOver], 0       ;ignore if not 1 / set
	je   .done
	mov  byte [gameOver], 0
	jmp  .done

.esc:
	mov eax, 0
	jmp start.returnToShell
.done:
	ret	

delay:
	mov  ecx, 20000000
.loop:
	dec  ecx
	jnz  .loop
	ret

update_position:

	movzx ecx, byte [score]
	test ecx, ecx
	jz .move_head ; score = 0

.shift_loop:
	;each segment is 8 bytes
	; need to move body[n-1] into body[n]
	mov eax, [body + ecx * 8 - 8] ; prev x
	mov [body + ecx * 8], eax ;current x

	mov eax, [body + ecx * 8 - 4] ; prev y
	mov [body + ecx * 8 + 4], eax ;current y

	loop .shift_loop
	
	mov eax, [xPos]
	mov [body], eax
	mov eax, [yPos]
	mov [body + 4], eax

.move_head:
	mov  al, [curDirection]
	cmp  al, 0
	je   .up

	cmp  al, 1
	je   .left

	cmp  al, 2
	je   .down

	cmp  al, 3
	je   .right

	ret

.up:
	dec  dword [yPos]
	ret

.down:
	inc  dword [yPos]
	ret

.left:
	dec  dword [xPos]
	ret

.right:
	inc  dword [xPos]
	ret

draw_player:
	call init_screen
	call draw_border
	
	mov ecx, score

	mov  eax, [yPos]
	mov  ebx, 160
	mul  ebx

	mov  ebx, [xPos]
	shl  ebx, 1

	add  eax, ebx
	add  eax, 0xB8000

	mov  byte [eax], '@'
	mov  byte [eax+1], 0x0A

	movzx ecx, byte [score]
	test ecx, ecx
	jz .done ; score = 0
	xor esi, esi

.draw_body_loop:
	push ecx
	; Calculate offset: y * 160 + x * 2
	mov   eax, [body + esi*8 + 4] ; Get Y of this segment
	mov   ebx, 160
	mul   ebx
	mov   ebx, [body + esi*8]     ; Get X of this segment
	shl   ebx, 1
	add   eax, ebx
	add   eax, 0xB8000

	mov   byte [eax], '@' 
	mov   byte [eax+1], 0x02      ; Green

	inc   esi
	pop   ecx
	loop  .draw_body_loop
.done:
	ret

init_screen:
	;pushad
	xor  ebx, ebx
	mov  ecx, 2000
.draw:
	mov  byte [0xB8000 + ebx], ' '
	mov  byte [0xB8001 + ebx], 0x00
	add  ebx, 2
	loop .draw
	;popad
	ret

draw_border:
	mov  ecx, 60                 ;60  characters
	mov  ebx, 20                 ; 10 spaces forward (offset)
.top_border:
	mov  byte [0xB8000 + ebx], 0xCD ; top pipe character
	mov  byte [0xB8001 + ebx], 0x02 ; black background white foreground
	add  ebx, 2
	loop .top_border

	mov  ebx, 20                 ;left side border position
	mov  ecx, 25
.left_border:
	mov  byte [0xB8000 + ebx], 0xBA ; straight pipe character
	mov  byte [0xB8001 + ebx], 0x02
	add  ebx, 160                ;next line
	loop .left_border

	mov  ebx, 140                ;right side border position
	mov  ecx, 25                 ;repetitions
.right_border:
	mov  byte [0xB8000 + ebx], 0xBA
	mov  byte [0xB8001 + ebx], 0x02
	add  ebx, 160
	loop .right_border

	mov  ebx, 3860               ; beginning of last line + offset
	mov  ecx, 60
.bottom_border:
	mov  byte [0xB8000 + ebx], 0xCD
	mov  byte [0xB8001 + ebx], 0x02
	add  ebx, 2
	loop .bottom_border
	ret

spawn_apple:
	.getCoords:
		rdtsc
		xor  edx, edx
		mov  ecx, 80
		div  ecx              ; edx = random x (0–79)

		mov  ebx, edx         ; save x in ebx

		; x bounds: 11–69
		cmp  ebx, 11
		jl   .getCoords
		cmp  ebx, 69
		jg   .getCoords

		rdtsc
		xor  edx, edx
		mov  ecx, 25
		div  ecx              ; edx = random y (0–24)

		mov  eax, edx         ; y in eax

		; y bounds: 1–23
		cmp  eax, 1
		jl   .getCoords
		cmp  eax, 23
		jg   .getCoords

		mov  ecx, 80
		mul  ecx              ; eax = y * 80
		add  eax, ebx         ; eax = y*80 + x

		mov  [applePos], eax
		ret

draw_apple:
	;NOTE need to check that this does not write over character
	mov  eax, [applePos]
	shl  eax, 1
	mov  byte [0xB8000 + eax], 153 ;apple character
	mov  byte [0xB8001 + eax], 0x04
	ret

collisions:
	; check if curPos = apple
	; check if curPos = border

	; curPos = yPos * 80 + xPos
	mov  eax, [yPos]
	mov  ebx, 80
	mul  ebx
	add  eax, [xPos]

	cmp  eax, [applePos]
	je   .collision_apple

	cmp  dword [xPos], 10
	jle  .collision_border

	cmp  dword [xPos], 70
	jae  .collision_border

	cmp  dword [yPos], 0
	je   .collision_border

	cmp  dword [yPos], 24
	je   .collision_border

	ret

.collision_apple:
	call spawn_apple
	inc  byte [score]
	ret

.collision_border:
	call game_over
	ret
	
print:
	; expects esi containing message
	; and expects edi to contain coordinates
	; ex. 
	; mov esi, gameOverMessage
	; mov edi, 0xB8000 + center
	cld
.loop:
	lodsb
	test al, al                    ;checks for 0 terminator
	jz   .done

	mov  [edi], al
	mov  byte [edi+1], 0x0F
	add  edi, 2
	jmp  .loop

.done:
	ret

game_over:
	inc  byte [gameOver]
	call init_screen
	mov  esi, gameOverMessage
	mov  edi, 0xB8000 + center - 22

	call print

	mov esi, gameOverInstructions
	mov edi, 0xB8000 + center + 106

	call print

.hang:	
	; calls read_input and waits for enter key to be pressed
	call read_input
	cmp  byte [gameOver], 0
	jne  .hang

	jmp  reset_game
	ret

reset_game:
	mov  dword [xPos], 40
	mov  dword [yPos], 12
	mov  byte [score], 0
	mov  byte [prevScore], 0
	mov  byte [curDirection], 3
	jmp start
	ret

draw_score:
	; prints "Score: XX" at top-left

	mov  esi, scoreMsg
	mov  edi, 0xB8000
	call print

	; edi now points right after "Score: "
	; print two-digit score

	movzx eax, byte [score]   ; load score (0– 255)
	xor  edx, edx
	mov  ecx, 10
	div  ecx                  ; eax = tens, edx = ones

	; print tens
	add  al, '0'
	mov  [edi], al
	mov  byte [edi+1], 0x0F
	add  edi, 2

	; print ones
	add  dl, '0'
	mov  [edi], dl
	mov  byte [edi+1], 0x0F

	ret
