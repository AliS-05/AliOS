 bits 16
  [org 0x7c00]

  KERNEL_SECTORS equ 384        ; 196608 bytes — must match the Makefile guard/padding

  start:
        ;init regs
        xor ax, ax
        mov ds, ax
        mov ss, ax
        mov es, ax
        mov sp, 0x7c00  ;stack below the bootloader, clear of the load region

        mov [boot_drive], dl

        ; ask BIOS for disk geometry (fallback: 63 spt, 16 heads)
        mov ah, 0x08
        mov dl, [boot_drive]
        xor di, di
        mov es, di
        int 0x13
        jc .got_geom
        mov al, cl
        and al, 0x3F
        xor ah, ah
        mov [spt], ax
        mov al, dh
        xor ah, ah
        inc ax
        mov [heads], ax
  .got_geom:
        xor ax, ax
        mov es, ax

        mov word [remaining], KERNEL_SECTORS
        mov word [cur_seg], 0x0800      ;0x0800:0000 = linear 0x8000
        mov word [cur_cyl], 0
        mov word [cur_head], 0

        ; first chunk: rest of track 0 (boot sector was sector 1)
        mov ax, [spt]
        dec ax
        mov cl, 2
        call read_chunk

  .track_loop:
        cmp word [remaining], 0
        je .load_done
        ; advance to next track
        mov ax, [cur_head]
        inc ax
        cmp ax, [heads]
        jb .same_cyl
        xor ax, ax
        inc word [cur_cyl]
  .same_cyl:
        mov [cur_head], ax
        mov ax, [spt]   ; whole track
        mov cl, 1       ; from sector 1
        call read_chunk
        jmp .track_loop

  .load_done:
        ; Enter Protected Mode
        cli

        in al, 0x92  ;flipping a20 line
        or al, 2
        out 0x92, al

        lgdt [gdtr]
        mov eax, cr0 ;flipping cr0 bit
        or eax, 1
        mov cr0, eax

        jmp 0x08:0x8000 ; far jumping to kernel

  ; reads AX sectors (capped to [remaining]) from [cur_cyl]/[cur_head], starting at sector CL,
  ; into [cur_seg]:0000, then advances cur_seg and remaining
  read_chunk:
        cmp ax, [remaining]
        jbe .count_ok
        mov ax, [remaining]
  .count_ok:
        test ax, ax
        jz .done
        push ax
        mov ch, [cur_cyl]       ; low 8 bits — fine, we never leave cylinder 0 at these sizes
        mov dh, [cur_head]
        mov dl, [boot_drive]
        mov bx, [cur_seg]
        mov es, bx
        xor bx, bx
        mov ah, 0x02            ; al already holds the count
        int 0x13
        jc disk_error
        pop ax
        sub [remaining], ax
        mov cx, ax
        shl cx, 5               ; sectors -> paragraphs (x32)
        add [cur_seg], cx
  .done:
        ret

  disk_error:
        mov si, disk_msg
  .print:
        lodsb
        test al, al
        jz .hang
        mov ah, 0x0E
        int 0x10
        jmp .print
  .hang:
        cli
        hlt
        jmp .hang

  disk_msg: db "DISK READ FAIL", 0
  boot_drive: db 0
  spt:       dw 63
  heads:     dw 16
  remaining: dw 0
  cur_seg:   dw 0
  cur_cyl:   dw 0
  cur_head:  dw 0

  gdt_start:
      dq 0
  gdt_code:
      dw 0xFFFF, 0
      db 0, 10011010b, 11001111b, 0
  gdt_data:
      dw 0xFFFF, 0
      db 0, 10010010b, 11001111b, 0
  gdt_end:
  gdtr:
      dw gdt_end - gdt_start - 1
      dd gdt_start

  times 510-($-$$) db 0
  dw 0xAA55 ;marking MBR
