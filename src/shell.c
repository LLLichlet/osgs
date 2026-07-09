/* osgs — shell: command-line interpreter with line editing. */

#include "shell.h"
#include "vga.h"
#include "keyboard.h"
#include "kernel.h"

#define CMD_BUF_SZ 128
#define MAX_CMDS   16

typedef void (*cmd_handler)(void);

typedef struct {
    const char *name;
    const char *desc;
    cmd_handler handler;
} cmd_entry;

static void help_handler(void);

static cmd_entry cmd_table[] = {
    {"help", "Show available commands", help_handler},
};
static int cmd_count = 1;

static char cmd_buf[CMD_BUF_SZ];

static void help_handler(void) {
    int i;
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

static const cmd_entry *find_cmd(const char *name) {
    int i;
    for (i = 0; i < cmd_count; ++i) {
        const char *a = name;
        const char *b = cmd_table[i].name;
        while (*a && *b && *a == *b) { ++a; ++b; }
        if (*a == '\0' && *b == '\0') {
            return &cmd_table[i];
        }
    }
    return NULL;
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
                cmd_buf[pos] = '\0';
                {
                    const cmd_entry *cmd = find_cmd(cmd_buf);
                    if (cmd) {
                        cmd->handler();
                    } else {
                        vga_set_attr(VGA_ATTR(VGA_LIGHT_RED, VGA_BLACK));
                        vga_puts("Unknown command: ");
                        vga_puts(cmd_buf);
                        vga_putc('\n');
                    }
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
