/* osgs — Pong: classic 2-player paddle game. */

#include "game.h"
#include "vga.h"
#include "keyboard.h"
#include "system.h"

/* scancodes */
#define KEY_ESC   0x01
#define KEY_W     0x11
#define KEY_S     0x1F
#define KEY_UP    0x48
#define KEY_DOWN  0x50

/* field boundaries */
#define FLD_L  1
#define FLD_R  78
#define FLD_T  2
#define FLD_B  23

#define PADDLE_L_X   2
#define PADDLE_R_X   77
#define PADDLE_H     4
#define WIN_SCORE    5

#define ATTR_SCORE VGA_ATTR(VGA_YELLOW, VGA_BLACK)
#define ATTR_BALL  VGA_ATTR(VGA_LIGHT_GREEN, VGA_BLACK)
#define ATTR_PADL  VGA_ATTR(VGA_LIGHT_CYAN, VGA_BLACK)
#define ATTR_PADR  VGA_ATTR(VGA_LIGHT_MAGENTA, VGA_BLACK)

static void put(int x, int y, char c, uint8_t attr) {
    vga_putc_at(x, y, c, attr);
}

static void draw_border(void) {
    int i;
    /* top border (row 1) */
    put(0, 1,  0xC9, VGA_DEFAULT_ATTR);
    put(79, 1, 0xBB, VGA_DEFAULT_ATTR);
    for (i = 1; i < 79; ++i) {
        put(i, 1, 0xCD, VGA_DEFAULT_ATTR);
    }
    /* bottom border (row 24) */
    put(0, 24, 0xC8, VGA_DEFAULT_ATTR);
    put(79, 24,0xBC, VGA_DEFAULT_ATTR);
    for (i = 1; i < 79; ++i) {
        put(i, 24, 0xCD, VGA_DEFAULT_ATTR);
    }
    /* side borders (rows 2..23) */
    for (i = FLD_T; i <= FLD_B; ++i) {
        put(0, i,  0xBA, VGA_DEFAULT_ATTR);
        put(79, i, 0xBA, VGA_DEFAULT_ATTR);
    }
    /* center net (dashed line) */
    for (i = FLD_T; i <= FLD_B; i += 2) {
        put(40, i, 0xB3, VGA_DEFAULT_ATTR);
    }
}

static void draw_score(int s1, int s2) {
    int x;
    char buf[20];
    char *digits = "0123456789";

    buf[0] = 'P'; buf[1] = '1'; buf[2] = ':'; buf[3] = ' ';
    buf[4] = digits[s1];
    buf[5] = ' ';
    buf[6] = ' ';
    buf[7] = ' ';
    buf[8] = 'P'; buf[9] = '2'; buf[10] = ':'; buf[11] = ' ';
    buf[12] = digits[s2];
    buf[13] = ' ';
    buf[14] = 'E'; buf[15] = 'S'; buf[16] = 'C';
    buf[17] = ' '; buf[18] = 'Q'; buf[19] = '\0';

    for (x = 0; x < 20; ++x) {
        put(30 + x, 0, buf[x], ATTR_SCORE);
    }
}

static void draw_paddle(int x, int y, uint8_t attr) {
    int i;
    for (i = 0; i < PADDLE_H; ++i) {
        put(x, y - 1 + i, 0xDB, attr);
    }
}

static void erase_paddle(int x, int y) {
    int i;
    for (i = 0; i < PADDLE_H; ++i) {
        put(x, y - 1 + i, ' ', VGA_DEFAULT_ATTR);
    }
}

static int sign(int n) {
    return (n > 0) ? 1 : -1;
}

void game_pong(void) {
    int p1_y, p2_y, p1_oy, p2_oy;
    int ball_x, ball_y, ball_ox, ball_oy;
    int ball_dx, ball_dy;
    int score1, score2;
    int tick;

    vga_clear();
    draw_border();
    p1_y = 13;
    p2_y = 13;
    ball_x = 40;
    ball_y = 13;
    ball_dx = 1;
    ball_dy = 1;
    score1 = 0;
    score2 = 0;
    tick = 0;

    draw_score(score1, score2);
    draw_paddle(PADDLE_L_X, p1_y, ATTR_PADL);
    draw_paddle(PADDLE_R_X, p2_y, ATTR_PADR);
    put(ball_x, ball_y, 0x07, ATTR_BALL);

    while (score1 < WIN_SCORE && score2 < WIN_SCORE) {
        uint8_t sc;

        /* input (non-blocking) */
        sc = kbd_get_scancode();
        if (sc == KEY_ESC) break;

        p1_oy = p1_y;
        p2_oy = p2_y;

        if (sc == KEY_W    && p1_y > FLD_T + 1)        --p1_y;
        if (sc == KEY_S    && p1_y < FLD_B - 2)        ++p1_y;
        if (sc == KEY_UP   && p2_y > FLD_T + 1)        --p2_y;
        if (sc == KEY_DOWN && p2_y < FLD_B - 2)        ++p2_y;

        /* redraw paddles only if moved */
        if (p1_y != p1_oy) {
            erase_paddle(PADDLE_L_X, p1_oy);
            draw_paddle(PADDLE_L_X, p1_y, ATTR_PADL);
        }
        if (p2_y != p2_oy) {
            erase_paddle(PADDLE_R_X, p2_oy);
            draw_paddle(PADDLE_R_X, p2_y, ATTR_PADR);
        }

        /* ball update (every 2nd tick) */
        ++tick;
        if (tick % 2 == 0) {
            ball_ox = ball_x;
            ball_oy = ball_y;

            ball_x += ball_dx;
            ball_y += ball_dy;

            /* bounce (ball enters border row before reversing) */
            if (ball_y < FLD_T) {
                ball_y = FLD_T + 1;
                ball_dy = 1;
            }
            if (ball_y > FLD_B) {
                ball_y = FLD_B - 1;
                ball_dy = -1;
            }

            /* bounce off left paddle */
            if (ball_x == PADDLE_L_X + 1 &&
                ball_y >= p1_y - 1 &&
                ball_y <= p1_y + 2) {
                ball_x = PADDLE_L_X + 1;
                ball_dx = 1;
                ball_dy = (ball_y > p1_y) ? 1 : ((ball_y < p1_y) ? -1 : 1);
            }

            /* bounce off right paddle */
            if (ball_x == PADDLE_R_X - 1 &&
                ball_y >= p2_y - 1 &&
                ball_y <= p2_y + 2) {
                ball_x = PADDLE_R_X - 1;
                ball_dx = -1;
                ball_dy = (ball_y > p2_y) ? 1 : ((ball_y < p2_y) ? -1 : -1);
            }

            /* scoring */
            if (ball_x < FLD_L) {
                ++score2;
                ball_x = 40; ball_y = 13;
                ball_dx = 1;
                ball_dy = (tick & 1) ? 1 : -1;
                draw_score(score1, score2);
            }
            if (ball_x > FLD_R) {
                ++score1;
                ball_x = 40; ball_y = 13;
                ball_dx = -1;
                ball_dy = (tick & 1) ? 1 : -1;
                draw_score(score1, score2);
            }

            /* restore old ball position */
            if (ball_ox >= 0 && ball_ox <= 79) {
                put(ball_ox, ball_oy,
                    (ball_oy == 1 || ball_oy == 24) ? (char)0xCD : ' ',
                    VGA_DEFAULT_ATTR);
            }
            /* draw new ball, clamp display to field */
            {
                int dx = ball_x;
                int dy = ball_y;
                if (dy < FLD_T) dy = FLD_T;
                if (dy > FLD_B) dy = FLD_B;
                put(dx, dy, 0x07, ATTR_BALL);
            }
        }

        sys_sleep(12);  /* ~83 fps, ball moves ~42 fps */
    }

    /* game over screen */
    vga_clear();
    if (score1 >= WIN_SCORE) {
        vga_set_attr(VGA_ATTR(VGA_LIGHT_GREEN, VGA_BLACK));
        vga_puts("Player 1 wins!\n");
    } else if (score2 >= WIN_SCORE) {
        vga_set_attr(VGA_ATTR(VGA_LIGHT_MAGENTA, VGA_BLACK));
        vga_puts("Player 2 wins!\n");
    }
    vga_set_attr(VGA_DEFAULT_ATTR);
    vga_puts("Final score: ");
    vga_putc('0' + score1);
    vga_puts(" - ");
    vga_putc('0' + score2);
    vga_putc('\n');
    vga_puts("Press any key to return...\n");
    kbd_getch();
}
