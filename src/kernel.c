/* osgs — kernel entry. Shows splash screen, then launches shell. */

#include "vga.h"
#include "keyboard.h"
#include "shell.h"

static void splash_str(int x, int y, const char *s, uint8_t attr) {
    while (*s) {
        vga_putc_at(x++, y, *s++, attr);
    }
}

static void splash_screen(void) {
    int x, y;
    int bx, by, bw, bh;
    uint8_t border, title, text, prompt;

    border = VGA_ATTR(VGA_LIGHT_GREEN, VGA_BLACK);
    title  = VGA_ATTR(VGA_LIGHT_CYAN, VGA_BLACK);
    text   = VGA_ATTR(VGA_LIGHT_GRAY, VGA_BLACK);
    prompt = VGA_ATTR(VGA_YELLOW, VGA_BLACK);

    bx = 10; by = 3;
    bw = 60; bh = 18;

    vga_clear();

    /* top border */
    vga_putc_at(bx, by, 0xC9, border);
    for (x = 1; x < bw - 1; ++x)
        vga_putc_at(bx + x, by, 0xCD, border);
    vga_putc_at(bx + bw - 1, by, 0xBB, border);

    /* side borders */
    for (y = 1; y < bh - 1; ++y) {
        vga_putc_at(bx, by + y, 0xBA, border);
        vga_putc_at(bx + bw - 1, by + y, 0xBA, border);
    }

    /* bottom border */
    vga_putc_at(bx, by + bh - 1, 0xC8, border);
    for (x = 1; x < bw - 1; ++x)
        vga_putc_at(bx + x, by + bh - 1, 0xCD, border);
    vga_putc_at(bx + bw - 1, by + bh - 1, 0xBC, border);

    /* content — center of box at col 40 */
    splash_str(38, by + 4,  "osgs", title);
    splash_str(28, by + 6,  "Old-School Games System", text);
    splash_str(38, by + 7,  "v1.0", text);
    splash_str(30, by + 9,  "A tiny real-mode OS", text);
    splash_str(32, by + 10, "for retro gaming.", text);

    /* decorative separator */
    vga_putc_at(36, by + 12, 0x10, text);
    splash_str(38, by + 12, " [+] ", title);
    vga_putc_at(44, by + 12, 0x11, text);

    splash_str(32, by + 15, "Press any key...", prompt);
}

void kmain(void) {
    vga_init();

    splash_screen();
    kbd_getch();  /* wait for any key */

    vga_clear();
    shell_run();

    while (1) {
        __asm { hlt }
    }
}
