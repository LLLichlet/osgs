; osgs boot sector — loads kernel (sectors 1-64) from floppy and jumps to it.

    org  0x7C00
    bits 16

start:
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

    ; load kernel: 63 sectors (31.5KB) from sectors 1-63 to 0x07E0:0x0000
    ; we read per-track to stay CHS-safe on real floppy controllers.
    ; track layout: 2 heads * 18 sectors/track = 36 sectors/cylinder.

    mov  ax, 0x07E0
    mov  es, ax

    ; track 0, head 0: sectors 2-18 (17 sectors)
    mov  ah, 0x02
    mov  al, 17
    mov  ch, 0x00
    mov  cl, 0x02
    mov  dh, 0x00
    mov  dl, 0x00
    xor  bx, bx
    int  0x13
    jc   disk_error

    ; track 0, head 1: sectors 1-18 (18 sectors)
    mov  ah, 0x02
    mov  al, 18
    mov  ch, 0x00
    mov  cl, 0x01
    mov  dh, 0x01
    add  bx, 17 * 512
    int  0x13
    jc   disk_error

    ; track 1, head 0: sectors 1-18 (18 sectors)
    mov  ah, 0x02
    mov  al, 18
    mov  ch, 0x01
    mov  cl, 0x01
    mov  dh, 0x00
    add  bx, 18 * 512
    int  0x13
    jc   disk_error

    ; track 1, head 1: sectors 1-10 (10 sectors)
    mov  ah, 0x02
    mov  al, 10
    mov  ch, 0x01
    mov  cl, 0x01
    mov  dh, 0x01
    add  bx, 18 * 512
    int  0x13
    jc   disk_error

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
