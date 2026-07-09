; osgs kernel entry — sets up segments, calls C kmain.

    segment CODE

    global _start
    global _small_code_
    extern kmain_

_start:
    mov  ax, 0x07E0
    mov  ds, ax
    mov  es, ax
    mov  ss, ax
    mov  sp, 0xFFFE
    call kmain_
    cli
    hlt

_small_code_ dw 0
