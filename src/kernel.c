/* osgs — kernel entry. Initializes hardware and enters main loop. */

#include "vga.h"

void kmain(void) {
    vga_init();

    vga_set_attr(VGA_ATTR(VGA_LIGHT_GREEN, VGA_BLACK));
    vga_puts("osgs - Old-School Games System\n");

    vga_set_attr(VGA_DEFAULT_ATTR);
    vga_puts("Kernel loaded.\n");
    vga_puts("Boot OK.\n\n");

    /* TODO: enter shell loop */
    while (1) {
        __asm { hlt }
    }
}
