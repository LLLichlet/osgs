/* osgs — shell: command-line interpreter with line editing. */

#include "shell.h"
#include "vga.h"
#include "keyboard.h"
#include "game.h"

#define CMD_BUF_SZ 128
#define CMD_NAME_SZ 32

typedef void (*cmd_handler)(const char *args);

typedef struct {
    const char *name;
    const char *desc;
    cmd_handler handler;
} cmd_entry;

static void help_handler(const char *args);
static void cls_handler(const char *args);
static void reboot_handler(const char *args);
static void list_handler(const char *args);
static void run_handler(const char *args);
static void about_handler(const char *args);

static cmd_entry cmd_table[] = {
    {"help",   "Show available commands",   help_handler},
    {"cls",    "Clear the screen",          cls_handler},
    {"list",   "List available games",      list_handler},
    {"run",    "Run a game (run <name>)",   run_handler},
    {"about",  "About this system",         about_handler},
    {"reboot", "Reboot the system",         reboot_handler},
};
static int cmd_count = 6;

static char cmd_buf[CMD_BUF_SZ];

/*
 * Extract command name (first word) from line into cmd_name.
 * Returns pointer to the rest of line (arguments), or empty string.
 */
static const char *parse_args(const char *line, char *cmd_name, int sz) {
    int i;
    /* skip leading spaces */
    while (*line == ' ') ++line;
    /* copy command name */
    i = 0;
    while (*line && *line != ' ' && i < sz - 1) {
        cmd_name[i++] = *line++;
    }
    cmd_name[i] = '\0';
    /* skip spaces after command name */
    while (*line == ' ') ++line;
    return line;
}

static const cmd_entry *find_cmd(const char *name) {
    int i;
    for (i = 0; i < cmd_count; ++i) {
        const char *a = name;
        const char *b = cmd_table[i].name;
        while (*a && *b && *a == *b) { ++a; ++b; }
        if (*a == '\0' && *b == '\0') return &cmd_table[i];
    }
    return NULL;
}

static void help_handler(const char *args) {
    int i;
    (void)args;
    vga_set_attr(VGA_ATTR(VGA_LIGHT_CYAN, VGA_BLACK));
    vga_puts("Available commands:\n");
    vga_set_attr(VGA_DEFAULT_ATTR);
    for (i = 0; i < cmd_count; ++i) {
        vga_puts("  ");
        vga_puts(cmd_table[i].name);
        vga_puts(" - ");
        vga_puts(cmd_table[i].desc);
        vga_putc('\n');
    }
}

static void cls_handler(const char *args) {
    (void)args;
    vga_clear();
}

static void about_handler(const char *args) {
    (void)args;
    vga_set_attr(VGA_ATTR(VGA_LIGHT_GREEN, VGA_BLACK));
    vga_puts("  osgs - Old-School Games System\n");
    vga_puts("  version 0.3.0\n");
    vga_set_attr(VGA_DEFAULT_ATTR);
    vga_puts("  A tiny real-mode OS for retro gaming.\n");
    vga_puts("  Built with OpenWatcom C + NASM.\n");
    vga_puts("  https://github.com/LLLichlet/osgs\n");
}

static void reboot_handler(const char *args) {
    (void)args;
    vga_puts("Rebooting...\n");
    __asm { int 19h }
}

static void list_handler(const char *args) {
    (void)args;
    vga_set_attr(VGA_ATTR(VGA_LIGHT_CYAN, VGA_BLACK));
    vga_puts("Games:\n");
    vga_set_attr(VGA_DEFAULT_ATTR);
    game_list();
}

static void run_handler(const char *args) {
    if (*args == '\0') {
        vga_set_attr(VGA_ATTR(VGA_LIGHT_RED, VGA_BLACK));
        vga_puts("Usage: run <game>\n");
        return;
    }
    if (game_run(args) != 0) {
        vga_set_attr(VGA_ATTR(VGA_LIGHT_RED, VGA_BLACK));
        vga_puts("No such game: ");
        vga_puts(args);
        vga_putc('\n');
    }
}

void shell_run(void) {
    int pos;
    vga_set_attr(VGA_ATTR(VGA_LIGHT_GREEN, VGA_BLACK));
    vga_puts("OSGS> ");
    vga_set_attr(VGA_DEFAULT_ATTR);
    pos = 0;

    while (1) {
        char c = kbd_getch();

        if (c == '\r') {
            vga_putc('\n');
            if (pos > 0) {
                char cmd_name[CMD_NAME_SZ];
                const char *args;
                const cmd_entry *cmd;

                cmd_buf[pos] = '\0';
                args = parse_args(cmd_buf, cmd_name, sizeof(cmd_name));
                cmd = find_cmd(cmd_name);
                if (cmd) {
                    cmd->handler(args);
                } else {
                    vga_set_attr(VGA_ATTR(VGA_LIGHT_RED, VGA_BLACK));
                    vga_puts("Unknown command: ");
                    vga_puts(cmd_name);
                    vga_putc('\n');
                }
            }
            pos = 0;
            vga_set_attr(VGA_ATTR(VGA_LIGHT_GREEN, VGA_BLACK));
            vga_puts("OSGS> ");
            vga_set_attr(VGA_DEFAULT_ATTR);
        } else if (c == '\b') {
            if (pos > 0) {
                --pos;
                vga_putc('\b');
            }
        } else if (c >= ' ' && pos < CMD_BUF_SZ - 1) {
            cmd_buf[pos++] = c;
            vga_putc(c);
        }
    }
}
