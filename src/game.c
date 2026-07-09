/* osgs — game manager. Central registry of all games. */

#include "game.h"
#include "vga.h"

/* forward declarations */
void game_pong(void);

static game_entry games[] = {
    {"pong", "Classic Pong - 2 Player", game_pong},
};
static int game_cnt = 1;

void game_list(void) {
    int i;
    for (i = 0; i < game_cnt; ++i) {
        vga_puts("  ");
        vga_puts(games[i].name);
        vga_puts(" - ");
        vga_puts(games[i].desc);
        vga_putc('\n');
    }
}

int game_run(const char *name) {
    int i;
    for (i = 0; i < game_cnt; ++i) {
        const char *a = name;
        const char *b = games[i].name;
        while (*a && *b && *a == *b) { ++a; ++b; }
        if (*a == '\0' && *b == '\0') {
            games[i].run();
            return 0;
        }
    }
    return -1;
}
