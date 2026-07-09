/* osgs — keyboard input via BIOS INT 16h. */

#include "keyboard.h"

bool kbd_kbhit(void) {
    char has_key;
    has_key = 0;
    __asm {
        mov ah, 01h
        int 16h
        jz  done
        mov has_key, 1
    done:
    }
    return has_key;
}

char kbd_getch(void) {
    char c;
    __asm {
        mov ah, 00h
        int 16h
        mov c, al
    }
    return c;
}

uint8_t kbd_get_scancode(void) {
    uint8_t sc;
    sc = 0;
    __asm {
        mov ah, 01h
        int 16h
        jz  done
        mov ah, 00h
        int 16h
        mov sc, ah
    done:
    }
    return sc;
}
