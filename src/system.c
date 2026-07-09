/* osgs — system utilities. */

#include "system.h"

void sys_sleep(int ms) {
    while (ms > 0) {
        __asm {
            mov ah, 86h
            mov cx, 0
            mov dx, 1000    ; 1000 us = 1 ms
            int 15h
        }
        --ms;
    }
}
