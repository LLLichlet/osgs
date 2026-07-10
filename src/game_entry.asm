; osgs — game binary entry point. Kernel far-calls here; we far-return when done.

    segment CODE

    global _start
    global _small_code_
    extern game_main_

_start:
    mov  ax, cs          ; align DS/ES/SS to game's code segment
    mov  ds, ax
    mov  es, ax
    mov  ss, ax
    mov  sp, 0xFFFE
    call game_main_
    retf                 ; far return to kernel

_small_code_ dw 0
