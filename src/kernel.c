/* osgs — kernel entry. Initializes hardware and enters main loop. */

#include "vga.h"
#include "keyboard.h"

void kmain(void) {
    vga_init();

    vga_set_attr(VGA_ATTR(VGA_LIGHT_GREEN, VGA_BLACK));
    vga_puts("osgs - Old-School Games System\n");

    vga_set_attr(VGA_DEFAULT_ATTR);
    vga_puts("Kernel loaded. Boot OK.\n");

    /* echo test — type ESC to halt */
    vga_puts("Type something (ESC to halt):\n");
    while (1) {
        char c = kbd_getch();
        if (c == 27) { /* ESC */
            vga_set_attr(VGA_ATTR(VGA_YELLOW, VGA_BLACK));
            vga_puts("[ESC] Halted.\n");
            break;
        }
        if (c == '\r') {
            c = '\n';  /* Enter -> newline */
        }
        vga_putc(c);
    }

    while (1) {
        __asm { hlt }
    }
}
