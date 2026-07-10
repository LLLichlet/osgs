; osgs — game binary entry point. Kernel far-calls here; we far-return when done.

    segment CODE

    global _start
    global _small_code_
    extern game_main_

_start:
    mov  ax, cs           ; ax = game segment (0x2000)

    ; rescue kernel DS before switching
    mov  bx, ds           ; bx = kernel DS (0x07E0)
    mov  ds, ax           ; DS = game segment (safe to access [saved_*])
    mov  [saved_ds], bx   ; stash kernel DS in game memory
    mov  es, ax

    ; save kernel stack
    mov  [saved_ss], ss
    mov  [saved_sp], sp

    ; rescue far return address from kernel's stack
    pop  bx              ; kernel IP
    pop  cx              ; kernel CS

    ; switch to game's own stack
    mov  ss, ax
    mov  sp, 0xFFFE

    ; stash return address
    mov  [saved_ip], bx
    mov  [saved_cs], cx

    call game_main_

    ; load return address (while DS is still game segment)
    mov  bx, [saved_ip]
    mov  cx, [saved_cs]

    ; restore kernel's stack
    mov  ss, [saved_ss]
    mov  sp, [saved_sp]

    ; push return address onto kernel's stack
    push cx
    push bx

    ; restore kernel's DS so kernel code finds its own data
    mov  ds, [saved_ds]

    retf

_small_code_ dw 0

saved_ds: dw 0
saved_ss: dw 0
saved_sp: dw 0
saved_ip: dw 0
saved_cs: dw 0
