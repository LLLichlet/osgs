/* osgs — VGA text mode driver. Writes directly to video memory at B800:0000. */

#include "vga.h"

/* far pointer to VGA text buffer (segment B800, offset 0000) */
static volatile uint16_t __far * const VGA_BUF =
    (volatile uint16_t __far *)0xB8000000UL;

static int cursor_x = 0;
static int cursor_y = 0;
static uint8_t current_attr = VGA_DEFAULT_ATTR;

/* port I/O helpers */
static void outb(uint16_t port, uint8_t val) {
    port = port;
    val = val;
    __asm {
        mov dx, port
        mov al, val
        out dx, al
    }
}

static uint8_t inb(uint16_t port) {
    uint8_t result;
    port = port;
    __asm {
        mov dx, port
        in  al, dx
        mov result, al
    }
    return result;
}

void vga_init(void) {
    /* set text mode 3 (80x25, 16-color) via BIOS — harmless if already in it,
     * critical for restoring text mode after a game used graphics mode */
    __asm {
        mov ax, 0003h
        int 10h
    }
    vga_clear();
}

void vga_clear(void) {
    uint16_t blank = (uint16_t)((current_attr << 8) | ' ');
    int i;
    for (i = 0; i < VGA_COLS * VGA_ROWS; ++i) {
        VGA_BUF[i] = blank;
    }
    cursor_x = 0;
    cursor_y = VGA_ROWS - 1;  /* start at bottom row */
    vga_set_cursor(0, cursor_y);
}

void vga_putc(char c) {
    if (c == '\n') {
        vga_scroll();        /* shift everything up, clear bottom */
        cursor_x = 0;
        /* cursor_y stays at bottom */
    } else if (c == '\r') {
        cursor_x = 0;
    } else if (c == '\b') {
        if (cursor_x > 0) {
            --cursor_x;
            VGA_BUF[cursor_y * VGA_COLS + cursor_x] =
                (uint16_t)((current_attr << 8) | ' ');
        }
    } else {
        int idx = cursor_y * VGA_COLS + cursor_x;
        VGA_BUF[idx] = (uint16_t)((current_attr << 8) | (uint8_t)c);
        ++cursor_x;
    }

    if (cursor_x >= VGA_COLS) {
        vga_scroll();
        cursor_x = 0;
    }

    vga_set_cursor(cursor_x, cursor_y);
}

void vga_puts(const char *s) {
    while (*s) {
        vga_putc(*s++);
    }
}

void vga_putc_at(int x, int y, char c, uint8_t attr) {
    VGA_BUF[y * VGA_COLS + x] = (uint16_t)((attr << 8) | (uint8_t)c);
}

void vga_set_attr(uint8_t attr) {
    current_attr = attr;
}

void vga_set_cursor(int x, int y) {
    uint16_t pos = (uint16_t)(y * VGA_COLS + x);
    /* CRT controller: select cursor location high byte */
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)(pos >> 8));
    /* CRT controller: select cursor location low byte */
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
}

void vga_scroll(void) {
    int x, y;
    uint16_t blank = (uint16_t)((current_attr << 8) | ' ');
    /* move each row up by one */
    for (y = 0; y < VGA_ROWS - 1; ++y) {
        for (x = 0; x < VGA_COLS; ++x) {
            VGA_BUF[y * VGA_COLS + x] = VGA_BUF[(y + 1) * VGA_COLS + x];
        }
    }
    /* clear last row */
    for (x = 0; x < VGA_COLS; ++x) {
        VGA_BUF[(VGA_ROWS - 1) * VGA_COLS + x] = blank;
    }
}
