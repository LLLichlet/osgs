/* osgs — game manager: lists TOC entries, loads & launches game binaries. */

#ifndef GAME_H
#define GAME_H

void game_list(void);
int  game_run(const char *name);   /* 0 = ok, -1 = not found, -2 = disk error */

#endif
