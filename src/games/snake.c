/* osgs — Snake: classic arcade game */

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
#define KEY_A     0x1E
#define KEY_D     0x20
#define KEY_LEFT  0x4B
#define KEY_RIGHT 0x4D

/* layout */
#define SCORE_H   14
#define BORDER_W  1
#define TILE_SZ   5
#define GRID_COLS 60
#define GRID_ROWS 35
#define FLD_X     ((GFX_W - GRID_COLS * TILE_SZ) / 2)    /* = 10 */
#define FLD_Y     (SCORE_H + 2)

#define MAX_LEN   200
#define INIT_LEN  4

/* colors */
#define C_BG      GFX_BLACK
#define C_BORDER  GFX_LIGHT_CYAN
#define C_SNAKE   GFX_GREEN
#define C_HEAD    GFX_LIGHT_GREEN
#define C_FOOD    GFX_YELLOW
#define C_SCORE   GFX_WHITE
#define C_OVER    GFX_LIGHT_RED

typedef struct {
    uint8_t x, y;
} point;

static point   snake[MAX_LEN];
static uint8_t head;
static uint8_t len;
static int8_t  dir_x, dir_y;
static uint8_t food_x, food_y;
static uint16_t rng;
static uint16_t score;

static int gx(uint8_t col) { return FLD_X + (int)col * TILE_SZ; }
static int gy(uint8_t row) { return FLD_Y + (int)row * TILE_SZ; }

static uint8_t rnd(uint8_t max) {
    rng = rng * 25173 + 13849;
    return (uint8_t)(((uint16_t)(rng >> 8) * max) >> 8);
}

static void draw_border(void) {
    gfx_rect(FLD_X - BORDER_W, FLD_Y - BORDER_W,
             GRID_COLS * TILE_SZ + BORDER_W * 2,
             GRID_ROWS * TILE_SZ + BORDER_W * 2, C_BORDER);
}

static void draw_score(void) {
    char buf[12];
    uint16_t n;
    int pos;

    gfx_drawstr(FLD_X, 3, "SCORE: ", C_SCORE, C_BG);
    n = score;
    pos = 10;
    if (n == 0) buf[--pos] = '0';
    else
        while (n && pos > 5) { buf[--pos] = '0' + (char)(n % 10); n /= 10; }
    buf[10] = '\0';
    gfx_drawstr(FLD_X + 6 * 8, 3, &buf[pos], C_SCORE, C_BG);
}

static uint8_t tail_idx(void) {
    return (uint8_t)((head + MAX_LEN - len + 1) % MAX_LEN);
}

static int is_on_snake(uint8_t gx_, uint8_t gy_, int skip_head) {
    uint8_t i, idx, start;
    start = skip_head ? 1 : 0;
    for (i = start; i < len; ++i) {
        idx = (uint8_t)((head + MAX_LEN - i) % MAX_LEN);
        if (snake[idx].x == gx_ && snake[idx].y == gy_) return 1;
    }
    return 0;
}

static void place_food(void) {
    do {
        food_x = rnd((uint8_t)GRID_COLS);
        food_y = rnd((uint8_t)GRID_ROWS);
    } while (is_on_snake(food_x, food_y, 0));
}

static void draw_seg(uint8_t sx, uint8_t sy, uint8_t color) {
    gfx_fill(gx(sx), gy(sy), TILE_SZ, TILE_SZ, color);
}

static void draw_snake_all(uint8_t head_color, uint8_t body_color) {
    uint8_t i, idx;
    for (i = 0; i < len; ++i) {
        idx = (uint8_t)((head + MAX_LEN - i) % MAX_LEN);
        draw_seg(snake[idx].x, snake[idx].y, (i == 0) ? head_color : body_color);
    }
}

int game_main(void) {
    uint8_t new_head, old_tail, prev_head;
    uint8_t nx, ny;
    uint8_t running, dead;
    uint8_t sc;
    uint16_t delay;
    uint8_t i;
    int flashes;

    gfx_init();
    gfx_clear(C_BG);
    draw_border();

    /* init snake horizontal, centered */
    dir_x = 1; dir_y = 0;
    len  = INIT_LEN;
    head = INIT_LEN - 1;
    {
        uint8_t sx = (uint8_t)(GRID_COLS / 2);
        uint8_t sy = (uint8_t)(GRID_ROWS / 2);
        for (i = 0; i < len; ++i) {
            snake[len - 1 - i].x = (uint8_t)(sx - i);
            snake[len - 1 - i].y = sy;
        }
    }
    score = 0;
    rng   = 0xACE1;

    draw_score();
    place_food();
    draw_seg(food_x, food_y, C_FOOD);
    draw_snake_all(C_HEAD, C_SNAKE);

    running = 1;
    dead    = 0;

    while (running) {
        do {
            sc = kbd_get_scancode();
            if (sc == KEY_ESC) { running = 0; dead = 0; break; }
            if ((sc == KEY_W    || sc == KEY_UP)    && dir_y != 1)  { dir_x = 0;  dir_y = -1; }
            if ((sc == KEY_S    || sc == KEY_DOWN)  && dir_y != -1) { dir_x = 0;  dir_y = 1;  }
            if ((sc == KEY_A    || sc == KEY_LEFT)  && dir_x != 1)  { dir_x = -1; dir_y = 0;  }
            if ((sc == KEY_D    || sc == KEY_RIGHT) && dir_x != -1) { dir_x = 1;  dir_y = 0;  }
        } while (sc);

        if (!running) break;

        nx = snake[head].x + dir_x;
        ny = snake[head].y + dir_y;

        if (nx >= GRID_COLS || ny >= GRID_ROWS) { dead = 1; break; }
        if (is_on_snake(nx, ny, 1))            { dead = 1; break; }

        prev_head = head;
        old_tail  = tail_idx();
        new_head  = (uint8_t)((head + 1) % MAX_LEN);
        head = new_head;
        snake[head].x = nx;
        snake[head].y = ny;

        if (nx == food_x && ny == food_y) {
            ++len;
            ++score;
            draw_score();
            draw_seg(snake[prev_head].x, snake[prev_head].y, C_SNAKE);
            draw_seg(nx, ny, C_HEAD);
            if (len < MAX_LEN) {
                place_food();
                draw_seg(food_x, food_y, C_FOOD);
            }
        } else {
            draw_seg(snake[old_tail].x, snake[old_tail].y, C_BG);
            draw_seg(snake[prev_head].x, snake[prev_head].y, C_SNAKE);
            draw_seg(nx, ny, C_HEAD);
        }

        /* faster pace to match higher resolution */
        if      (score < 10) delay = 30;
        else if (score < 25) delay = 18;
        else if (score < 40) delay = 10;
        else                 delay = 6;
        sys_sleep((int)delay);
    }

    if (dead) {
        for (flashes = 0; flashes < 3; ++flashes) {
            draw_snake_all(GFX_LIGHT_RED, GFX_RED);
            sys_sleep(150);
            draw_snake_all(C_HEAD, C_SNAKE);
            sys_sleep(100);
        }
    }

    /* game over screen (text mode) */
    gfx_shutdown();
    vga_clear();
    vga_set_attr(VGA_ATTR(VGA_LIGHT_RED, VGA_BLACK));
    vga_puts("Game Over!\n\n");
    vga_set_attr(VGA_DEFAULT_ATTR);
    vga_puts("Final score: ");
    {
        uint16_t n = score;
        char digits[6];
        int di = 0;
        if (n == 0) {
            vga_putc('0');
        } else {
            while (n > 0 && di < 5) { digits[di++] = '0' + (char)(n % 10); n /= 10; }
            while (di > 0) vga_putc(digits[--di]);
        }
    }
    vga_puts("\nPress any key to return...");
    while (kbd_kbhit()) kbd_getch();
    kbd_getch();
    return 0;
}
