/* osgs — Pong: classic 2-player paddle game. (placeholder) */

#include "game.h"
#include "vga.h"
#include "keyboard.h"

void game_pong(void) {
    vga_clear();
    vga_set_attr(VGA_ATTR(VGA_LIGHT_CYAN, VGA_BLACK));
    vga_puts("=== Pong ===\n");
    vga_set_attr(VGA_DEFAULT_ATTR);
    vga_puts("Coming soon!\n\n");
    vga_puts("Press any key to return...\n");
    kbd_getch();
}
