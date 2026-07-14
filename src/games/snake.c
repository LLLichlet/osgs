/* osgs — Snake: classic arcade game. */

#include "vga.h"
#include "keyboard.h"
#include "system.h"

/* scancodes */
#define KEY_ESC   0x01
#define KEY_W     0x11
#define KEY_S     0x1F
#define KEY_UP    0x48
#define KEY_DOWN  0x50
#define KEY_A     0x1E
#define KEY_D     0x20
#define KEY_LEFT  0x4B
#define KEY_RIGHT 0x4D

/* playfield (inside border) */
#define FLD_L  1
#define FLD_R  78
#define FLD_T  2
#define FLD_B  23
#define FLD_W  (FLD_R - FLD_L + 1)
#define FLD_H  (FLD_B - FLD_T + 1)

#define MAX_LEN   200
#define INIT_LEN  4

/* tile chars */
#define SNAKE_CHAR 0xDB
#define FOOD_CHAR  0x0F

/* colors */
#define ATTR_SNAKE   VGA_ATTR(VGA_LIGHT_GREEN, VGA_BLACK)
#define ATTR_HEAD    VGA_ATTR(VGA_GREEN, VGA_LIGHT_GREEN)
#define ATTR_FOOD    VGA_ATTR(VGA_YELLOW, VGA_BLACK)
#define ATTR_BORDER  VGA_ATTR(VGA_LIGHT_CYAN, VGA_BLACK)
#define ATTR_SCORE   VGA_ATTR(VGA_WHITE, VGA_BLACK)
#define ATTR_OVER    VGA_ATTR(VGA_LIGHT_RED, VGA_BLACK)

typedef struct {
    uint8_t x, y;
} point;

static point   snake[MAX_LEN];
static uint8_t head;   /* ring-buffer head index */
static uint8_t len;
static int8_t  dir_x, dir_y;
static uint8_t food_x, food_y;
static uint16_t rng;
static uint16_t score;

static void put(int x, int y, char c, uint8_t attr) {
    vga_putc_at(x, y, c, attr);
}

static uint8_t rnd(uint8_t max) {
    rng = rng * 25173 + 13849;
    return (uint8_t)(((uint16_t)(rng >> 8) * max) >> 8);
}

static void draw_border(void) {
    int i;
    put(0, 1, 0xC9, ATTR_BORDER);
    put(79, 1, 0xBB, ATTR_BORDER);
    for (i = 1; i < 79; ++i) put(i, 1,  0xCD, ATTR_BORDER);
    put(0, 24, 0xC8, ATTR_BORDER);
    put(79, 24, 0xBC, ATTR_BORDER);
    for (i = 1; i < 79; ++i) put(i, 24, 0xCD, ATTR_BORDER);
    for (i = FLD_T; i <= FLD_B; ++i) {
        put(0, i,  0xBA, ATTR_BORDER);
        put(79, i, 0xBA, ATTR_BORDER);
    }
}

static void draw_score(void) {
    const char *s = "SCORE: ";
    uint16_t n;
    int i;
    char buf[6];
    for (i = 0; s[i]; ++i) put(2 + i, 0, s[i], ATTR_SCORE);

    n = score;
    /* up to 5 digits */
    i = 5;
    if (n == 0) buf[--i] = '0';
    while (n && i > 0) { buf[--i] = '0' + (n % 10); n /= 10; }
    while (i > 0) buf[--i] = ' ';
    for (i = 0; i < 5; ++i) put(9 + i, 0, buf[i], ATTR_SCORE);
}

/* ring-buffer helper: index of tail (oldest segment) */
static uint8_t tail_idx(void) {
    return (uint8_t)((head + MAX_LEN - len + 1) % MAX_LEN);
}

static int is_on_snake(uint8_t x, uint8_t y, int skip_head) {
    uint8_t i, idx, start;
    start = skip_head ? 1 : 0;
    for (i = start; i < len; ++i) {
        idx = (uint8_t)((head + MAX_LEN - i) % MAX_LEN);
        if (snake[idx].x == x && snake[idx].y == y) return 1;
    }
    return 0;
}

static void place_food(void) {
    do {
        food_x = (uint8_t)(FLD_L + 1 + rnd((uint8_t)(FLD_W - 3)));
        food_y = (uint8_t)(FLD_T + rnd((uint8_t)FLD_H));
    } while (is_on_snake(food_x, food_y, 0));
}

static void draw_all(void) {
    uint8_t i, idx;
    vga_clear();
    draw_border();
    draw_score();
    /* draw food */
    put(food_x, food_y, FOOD_CHAR, ATTR_FOOD);
    /* draw snake from tail to head */
    for (i = 0; i < len; ++i) {
        idx = (uint8_t)((head + MAX_LEN - i) % MAX_LEN);
        put(snake[idx].x, snake[idx].y, SNAKE_CHAR,
            (i == 0) ? ATTR_HEAD : ATTR_SNAKE);
    }
}

int game_main(void) {
    uint8_t new_head, old_tail;
    uint8_t nx, ny;
    uint8_t running, dead;
    uint8_t sc;
    uint16_t delay;
    uint8_t i, idx;
    int flashes;

    vga_clear();

    /* init snake: horizontal, center of field */
    dir_x = 1; dir_y = 0;
    len  = INIT_LEN;
    head = INIT_LEN - 1;
    {
        uint8_t sx = 40;
        uint8_t sy = 13;
        for (i = 0; i < len; ++i) {
            snake[len - 1 - i].x = (uint8_t)(sx - i);
            snake[len - 1 - i].y = sy;
        }
    }
    score = 0;
    rng   = 0xACE1;

    place_food();
    draw_all();

    running = 1;
    dead    = 0;

    while (running) {
        /* input (non-blocking) */
        do {
            sc = kbd_get_scancode();
            if (sc == KEY_ESC) { running = 0; dead = 0; break; }
            if ((sc == KEY_W    || sc == KEY_UP)    && dir_y != 1)  { dir_x = 0;  dir_y = -1; }
            if ((sc == KEY_S    || sc == KEY_DOWN)  && dir_y != -1) { dir_x = 0;  dir_y = 1;  }
            if ((sc == KEY_A    || sc == KEY_LEFT)  && dir_x != 1)  { dir_x = -1; dir_y = 0;  }
            if ((sc == KEY_D    || sc == KEY_RIGHT) && dir_x != -1) { dir_x = 1;  dir_y = 0;  }
        } while (sc);

        if (!running) break;

        /* compute new head position */
        nx = snake[head].x + dir_x;
        ny = snake[head].y + dir_y;

        /* wall collision */
        if (nx < FLD_L || nx > FLD_R || ny < FLD_T || ny > FLD_B) {
            dead = 1; break;
        }

        /* self collision (skip current head) */
        if (is_on_snake(nx, ny, 1)) {
            dead = 1; break;
        }

        /* remember old tail for erasing */
        old_tail = tail_idx();

        /* advance head in ring buffer */
        new_head = (uint8_t)((head + 1) % MAX_LEN);
        snake[new_head].x = nx;
        snake[new_head].y = ny;

        /* food check */
        if (nx == food_x && ny == food_y) {
            head = new_head;
            ++len;
            ++score;
            draw_score();
            put(food_x, food_y, SNAKE_CHAR, ATTR_SNAKE);
            put(nx, ny, SNAKE_CHAR, ATTR_HEAD);
            if (len < MAX_LEN) {
                place_food();
                put(food_x, food_y, FOOD_CHAR, ATTR_FOOD);
            }
        } else {
            put(snake[old_tail].x, snake[old_tail].y, ' ', VGA_DEFAULT_ATTR);
            head = new_head;
            if (len > 1) {
                uint8_t prev = (uint8_t)((head + MAX_LEN - 1) % MAX_LEN);
                put(snake[prev].x, snake[prev].y, SNAKE_CHAR, ATTR_SNAKE);
            }
            put(nx, ny, SNAKE_CHAR, ATTR_HEAD);
        }

        delay = (score < 10) ? 35 : (score < 25) ? 20 : (score < 40) ? 12 : 8;
        sys_sleep((int)delay);
    }

    /* flash snake on death only */
    if (dead) {
        for (flashes = 0; flashes < 3; ++flashes) {
            for (i = 0; i < len; ++i) {
                idx = (uint8_t)((head + MAX_LEN - i) % MAX_LEN);
                put(snake[idx].x, snake[idx].y, SNAKE_CHAR, ATTR_OVER);
            }
            sys_sleep(150);
            for (i = 0; i < len; ++i) {
                idx = (uint8_t)((head + MAX_LEN - i) % MAX_LEN);
                put(snake[idx].x, snake[idx].y, SNAKE_CHAR, ATTR_SNAKE);
            }
            sys_sleep(100);
        }
    }

    /* game over screen */
    vga_clear();
    vga_set_attr(ATTR_OVER);
    vga_puts("Game Over!\n\n");
    vga_set_attr(VGA_DEFAULT_ATTR);
    vga_puts("Final score: ");
    /* print score digit by digit */
    {
        uint16_t n = score;
        char digits[6];
        int di = 0;
        if (n == 0) {
            vga_putc('0');
        } else {
            while (n > 0 && di < 5) {
                digits[di++] = '0' + (char)(n % 10);
                n /= 10;
            }
            while (di > 0) {
                vga_putc(digits[--di]);
            }
        }
    }
    vga_puts("\nPress any key to return...");
    while (kbd_kbhit()) kbd_getch(); /* drain keys buffered during death flash */
    kbd_getch();
    return 0;
}
