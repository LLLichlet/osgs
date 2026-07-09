/* osgs — keyboard input via BIOS INT 16h. */

#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "kernel.h"

bool    kbd_kbhit(void);
char    kbd_getch(void);
uint8_t kbd_get_scancode(void);

#endif
