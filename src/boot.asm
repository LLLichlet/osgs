; osgs boot sector — loads kernel from floppy and jumps to it.

    org  0x7C00
    bits 16

start:
    ; set up segments and stack (SS:SP = 0x0000:0x7C00, stack below boot sector)
    xor  ax, ax
    mov  ds, ax
    mov  es, ax
    mov  ss, ax
    mov  sp, 0x7C00

    ; reset disk system
    mov  ah, 0x00
    mov  dl, 0x00
    int  0x13
    jc   disk_error

    ; load kernel: AH=02h (read sectors), AL=1 sector, CH=cyl, CL=sector, DH=head
    mov  ah, 0x02
    mov  al, 0x01
    mov  ch, 0x00
    mov  cl, 0x02
    mov  dh, 0x00
    mov  dl, 0x00
    mov  bx, 0x07E0
    mov  es, bx          ; ES:BX = 0x07E0:0x0000
    mov  bx, 0x0000
    int  0x13
    jc   disk_error

    ; far jump to kernel at 0x07E0:0x0000
    jmp  0x07E0:0x0000

disk_error:
    mov  si, err_msg
    call print_str
    cli
    hlt

print_str:
    lodsb
    or   al, al
    jz   .done
    mov  ah, 0x0E
    int  0x10
    jmp  print_str
.done:
    ret

err_msg db "Disk error!", 0

    times 510 - ($ - $$) db 0
    dw 0xAA55
