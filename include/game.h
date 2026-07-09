/* osgs — game manager: registry and launcher. */

#ifndef GAME_H
#define GAME_H

typedef void (*game_fn)(void);

typedef struct {
    const char *name;
    const char *desc;
    game_fn     run;
} game_entry;

void game_list(void);
int  game_run(const char *name);

#endif
