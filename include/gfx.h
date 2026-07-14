/* osgs — Mode 13h graphics driver (320x200, 256 colors). */

#ifndef GFX_HDR_H
#define GFX_HDR_H

#include "kernel.h"

#define GFX_W 320
#define GFX_H 200

/* Default VGA palette color indices (mode 13h reuses text-mode indices) */
#define GFX_BLACK         0x00
#define GFX_BLUE          0x01
#define GFX_GREEN         0x02
#define GFX_CYAN          0x03
#define GFX_RED           0x04
#define GFX_MAGENTA       0x05
#define GFX_BROWN         0x06
#define GFX_LIGHT_GRAY    0x07
#define GFX_DARK_GRAY     0x08
#define GFX_LIGHT_BLUE    0x09
#define GFX_LIGHT_GREEN   0x0A
#define GFX_LIGHT_CYAN    0x0B
#define GFX_LIGHT_RED     0x0C
#define GFX_LIGHT_MAGENTA 0x0D
#define GFX_YELLOW        0x0E
#define GFX_WHITE         0x0F

/* screen control */
void gfx_init(void);
void gfx_shutdown(void);
void gfx_clear(uint8_t color);

/* pixel / primitives */
void gfx_putpixel(int x, int y, uint8_t color);
void gfx_fill(int x, int y, int w, int h, uint8_t color);
void gfx_rect(int x, int y, int w, int h, uint8_t color);
void gfx_line(int x1, int y1, int x2, int y2, uint8_t color);

/* text (built-in 8x8 bitmap font, ASCII 32–126) */
void gfx_drawchar(int x, int y, char ch, uint8_t fg, uint8_t bg);
void gfx_drawstr(int x, int y, const char *s, uint8_t fg, uint8_t bg);

#endif
