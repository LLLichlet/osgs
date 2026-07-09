/* osgs — VGA text mode driver (80x25, 16 colors). */

#ifndef VGA_H
#define VGA_H

#include "kernel.h"

/* Color attributes: high nibble = background, low nibble = foreground */
#define VGA_BLACK         0x0
#define VGA_BLUE          0x1
#define VGA_GREEN         0x2
#define VGA_CYAN          0x3
#define VGA_RED           0x4
#define VGA_MAGENTA       0x5
#define VGA_BROWN         0x6
#define VGA_LIGHT_GRAY    0x7
#define VGA_DARK_GRAY     0x8
#define VGA_LIGHT_BLUE    0x9
#define VGA_LIGHT_GREEN   0xA
#define VGA_LIGHT_CYAN    0xB
#define VGA_LIGHT_RED     0xC
#define VGA_LIGHT_MAGENTA 0xD
#define VGA_YELLOW        0xE
#define VGA_WHITE         0xF

#define VGA_ATTR(fg, bg)  ((uint8_t)(((bg) << 4) | (fg)))
#define VGA_DEFAULT_ATTR   VGA_ATTR(VGA_LIGHT_GRAY, VGA_BLACK)

void vga_init(void);
void vga_clear(void);
void vga_putc(char c);
void vga_puts(const char *s);
void vga_putc_at(int x, int y, char c, uint8_t attr);
void vga_set_attr(uint8_t attr);
void vga_set_cursor(int x, int y);
void vga_scroll(void);

#endif
