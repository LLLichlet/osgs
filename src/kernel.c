/* osgs — kernel entry. Initializes hardware and launches shell. */

#include "vga.h"
#include "shell.h"

void kmain(void) {
    vga_init();

    vga_set_attr(VGA_ATTR(VGA_LIGHT_GREEN, VGA_BLACK));
    vga_puts("osgs - Old-School Games System\n");

    vga_set_attr(VGA_DEFAULT_ATTR);
    vga_puts("Kernel loaded. Boot OK.\n\n");

    shell_run();

    while (1) {
        __asm { hlt }
    }
}
