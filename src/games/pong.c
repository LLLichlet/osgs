/* osgs — Pong: classic 2-player paddle game */

#include "gfx.h"
#include "vga.h"
#include "keyboard.h"
#include "system.h"

/* scancodes */
#define KEY_ESC   0x01
#define KEY_W     0x11
#define KEY_S     0x1F
#define KEY_UP    0x48
#define KEY_DOWN  0x50

/* layout */
#define SCORE_H   14
#define BORDER_W  1
#define FLD_X     6
#define FLD_Y     (SCORE_H + 2)
#define FLD_W     308
#define FLD_H     180
#define FLD_R     (FLD_X + FLD_W - 1)
#define FLD_B     (FLD_Y + FLD_H - 1)
#define NET_X     (FLD_X + FLD_W / 2)

#define PADDLE_W  4
#define PADDLE_H  24
#define PADDLE_L_X (FLD_X + 6)
#define PADDLE_R_X (FLD_R - 6 - PADDLE_W)
#define PADDLE_INSET 3

#define BALL_SZ     4
#define BALL_SPEED  2
#define WIN_SCORE   5

/* colors */
#define C_BG     GFX_BLACK
#define C_BORDER GFX_WHITE
#define C_NET    GFX_DARK_GRAY
#define C_BALL   GFX_LIGHT_GREEN
#define C_PADL   GFX_LIGHT_CYAN
#define C_PADR   GFX_LIGHT_MAGENTA
#define C_SCORE  GFX_YELLOW

static void draw_border(void) {
    /* single-pixel box */
    gfx_rect(FLD_X - BORDER_W, FLD_Y - BORDER_W,
             FLD_W + BORDER_W * 2, FLD_H + BORDER_W * 2, C_BORDER);
}

static void draw_net(void) {
    int i;
    for (i = FLD_Y; i <= FLD_B; i += 8) {
        int h = i + 4 <= FLD_B + 1 ? 4 : FLD_B + 1 - i;
        if (h > 0) gfx_fill(NET_X, i, 1, h, C_NET);
    }
}

static void draw_score(int s1, int s2) {
    char buf[16];
    buf[0] = 'P'; buf[1] = '1'; buf[2] = ':'; buf[3] = ' ';
    buf[4] = '0' + s1;
    buf[5] = '\0';
    gfx_drawstr(FLD_X + 4, 3, buf, C_SCORE, C_BG);

    buf[0] = 'P'; buf[1] = '2'; buf[2] = ':'; buf[3] = ' ';
    buf[4] = '0' + s2;
    buf[5] = '\0';
    gfx_drawstr(FLD_R - 7 * 8 + 4, 3, buf, C_SCORE, C_BG);

    gfx_drawstr(NET_X - 12, 3, "ESC Q", GFX_DARK_GRAY, C_BG);
}

int game_main(void) {
    int p1_y, p2_y;
    int ball_x, ball_y, ball_dx, ball_dy;
    int score1, score2;

    gfx_init();
    gfx_clear(C_BG);
    draw_border();
    draw_net();

    p1_y = FLD_Y + FLD_H / 2 - PADDLE_H / 2;
    p2_y = FLD_Y + FLD_H / 2 - PADDLE_H / 2;
    ball_x = NET_X - BALL_SZ / 2;
    ball_y = FLD_Y + FLD_H / 2 - BALL_SZ / 2;
    ball_dx = BALL_SPEED;
    ball_dy = BALL_SPEED;
    score1 = 0;
    score2 = 0;

    draw_score(score1, score2);
    gfx_fill(PADDLE_L_X, p1_y, PADDLE_W, PADDLE_H, C_PADL);
    gfx_fill(PADDLE_R_X, p2_y, PADDLE_W, PADDLE_H, C_PADR);
    gfx_fill(ball_x, ball_y, BALL_SZ, BALL_SZ, C_BALL);

    while (score1 < WIN_SCORE && score2 < WIN_SCORE) {
        uint8_t sc;
        int old_bx = ball_x, old_by = ball_y;

        sc = kbd_get_scancode();
        if (sc == KEY_ESC) break;

        /* paddle movement */
        if (sc == KEY_W    && p1_y > FLD_Y + PADDLE_INSET)
            p1_y -= 5;
        if (sc == KEY_S    && p1_y < FLD_B - PADDLE_H - PADDLE_INSET)
            p1_y += 5;
        if (sc == KEY_UP   && p2_y > FLD_Y + PADDLE_INSET)
            p2_y -= 5;
        if (sc == KEY_DOWN && p2_y < FLD_B - PADDLE_H - PADDLE_INSET)
            p2_y += 5;

        /* ball — update every frame */
        ball_x += ball_dx;
        ball_y += ball_dy;

        /* top / bottom bounce */
        if (ball_y < FLD_Y) { ball_y = FLD_Y; ball_dy = -ball_dy; }
        if (ball_y + BALL_SZ - 1 > FLD_B) {
            ball_y = FLD_B - BALL_SZ + 1;
            ball_dy = -ball_dy;
        }

        /* left paddle collision */
        if (ball_dx < 0 &&
            ball_x <= PADDLE_L_X + PADDLE_W &&
            ball_x + BALL_SZ >= PADDLE_L_X &&
            ball_y + BALL_SZ >= p1_y &&
            ball_y <= p1_y + PADDLE_H) {
            ball_x = PADDLE_L_X + PADDLE_W;
            ball_dx = BALL_SPEED;
            {
                int hit = ball_y + BALL_SZ / 2 - p1_y;
                ball_dy = (hit - PADDLE_H / 2) / 4;
                if (ball_dy == 0) ball_dy = 1;
                if (ball_dy < -3) ball_dy = -3;
                if (ball_dy > 3)  ball_dy = 3;
            }
        }

        /* right paddle collision */
        if (ball_dx > 0 &&
            ball_x + BALL_SZ >= PADDLE_R_X &&
            ball_x <= PADDLE_R_X + PADDLE_W &&
            ball_y + BALL_SZ >= p2_y &&
            ball_y <= p2_y + PADDLE_H) {
            ball_x = PADDLE_R_X - BALL_SZ;
            ball_dx = -BALL_SPEED;
            {
                int hit = ball_y + BALL_SZ / 2 - p2_y;
                ball_dy = (hit - PADDLE_H / 2) / 4;
                if (ball_dy == 0) ball_dy = 1;
                if (ball_dy < -3) ball_dy = -3;
                if (ball_dy > 3)  ball_dy = 3;
            }
        }

        /* scoring */
        if (ball_x < FLD_X) {
            ++score2;
            ball_x = NET_X - BALL_SZ / 2;
            ball_y = FLD_Y + FLD_H / 2 - BALL_SZ / 2;
            ball_dx = BALL_SPEED;
            ball_dy = (ball_y & 1) ? 1 : -1;
            gfx_fill(0, 0, GFX_W, SCORE_H, C_BG);
            draw_score(score1, score2);
        }
        if (ball_x + BALL_SZ - 1 > FLD_R) {
            ++score1;
            ball_x = NET_X - BALL_SZ / 2;
            ball_y = FLD_Y + FLD_H / 2 - BALL_SZ / 2;
            ball_dx = -BALL_SPEED;
            ball_dy = (ball_y & 1) ? 1 : -1;
            gfx_fill(0, 0, GFX_W, SCORE_H, C_BG);
            draw_score(score1, score2);
        }

        /* erase old, draw new (partial update) */
        gfx_fill(old_bx, old_by, BALL_SZ, BALL_SZ, C_BG);
        gfx_fill(ball_x, ball_y, BALL_SZ, BALL_SZ, C_BALL);
        gfx_fill(PADDLE_L_X, FLD_Y, PADDLE_W, FLD_H, C_BG);
        gfx_fill(PADDLE_L_X, p1_y, PADDLE_W, PADDLE_H, C_PADL);
        gfx_fill(PADDLE_R_X, FLD_Y, PADDLE_W, FLD_H, C_BG);
        gfx_fill(PADDLE_R_X, p2_y, PADDLE_W, PADDLE_H, C_PADR);
        /* repair net dashes eaten by paddle/ball clears */
        {
            int i;
            for (i = FLD_Y; i <= FLD_B; i += 8) {
                int h = i + 4 <= FLD_B + 1 ? 4 : FLD_B + 1 - i;
                if (h > 0) gfx_fill(NET_X, i, 1, h, C_NET);
            }
        }

        sys_sleep(8);  /* ~125 fps */
    }

    /* game over screen (text mode) */
    gfx_shutdown();
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
    return 0;
}
