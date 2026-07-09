; osgs kernel entry — initializes segments and calls kmain.

    bits 16

start:
    ; re-initialize segment registers for kernel (CS=DS=ES=SS=0x07E0)
    mov  ax, 0x07E0
    mov  ds, ax
    mov  es, ax
    mov  ss, ax
    mov  sp, 0xFFFE      ; stack at top of 64KB segment

    ; print welcome via BIOS teletype (temporary, before VGA driver is up)
    mov  si, banner
    call print_str

    ; TODO: call kmain() once we have C compiler
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

banner db "osgs - Old-School Games System", 13, 10
       db "Kernel loaded successfully.", 13, 10, 0
