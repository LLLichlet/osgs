/* osgs — game manager. Loads games from TOC and far-calls into them. */

#include "game.h"
#include "toc.h"
#include "disk.h"
#include "vga.h"

#define GAME_SEG  0x2000
#define MK_FP(s, o) ((void far *)(((uint32_t)(s) << 16) | (uint16_t)(o)))

void game_list(void) {
    int i;
    int n = toc_count();
    for (i = 0; i < n; ++i) {
        const toc_entry *e = toc_get(i);
        if (!e) continue;
        vga_puts("  ");
        vga_puts(e->name);
        vga_puts(" - ");
        vga_puts(e->desc);
        vga_putc('\n');
    }
}

int game_run(const char *name) {
    const toc_entry *e;
    int err;

    e = toc_find(name);
    if (!e) return -1;

    err = disk_read(e->sector, e->nsectors, GAME_SEG, 0);
    if (err) return -2;

    /* TODO: decompress if e->flags & 1 */

    {
        void (far *fn)(void);
        fn = (void (far *)(void))MK_FP(GAME_SEG, 0);
        (*fn)();
    }

    /* game returned — restore VGA state */
    vga_init();
    return 0;
}
