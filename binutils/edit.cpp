// binutils/edit.cpp

#include <syscall.hpp>
#include <string.hpp>

constexpr size_t SCRIPT_MAX = 4096;
constexpr size_t LINE_MAX   = 128;

static char file_buf[SCRIPT_MAX];
static size_t file_len = 0;

static char edit_line[LINE_MAX];
static int edit_line_len = 0;
static int line_count = 0;

static void append_line(const char* line) {
    while (*line && file_len < SCRIPT_MAX - 2) {
        file_buf[file_len++] = *line++;
    }
    if (file_len < SCRIPT_MAX - 1) {
        file_buf[file_len++] = '\n';
    }
    file_buf[file_len] = 0;
}

extern "C" __attribute__((section(".text._start"))) void _start(const char* args) {
    if (!args || args[0] == 0) {
        sys_puts("Usage: edit <filename>\n");
        sys_exit();
    }

    sys_puts("[edit] editing '");
    sys_puts(args);
    sys_puts("'. Type '.save' to save, '.cancel' to exit.\n1: ");

    for (;;) {
        char c = sys_getchar();
        if (c == 0) {
            sys_yield();
            continue;
        }

        if (c == '\n') {
            edit_line[edit_line_len] = 0;

            if (str_equals(edit_line, ".save")) {
                sys_puts("\n");
                if (sys_write_file(args, file_buf, file_len)) {
                    sys_puts("[edit] saved ");
                    print_number(line_count);
                    sys_puts(" lines to ");
                    sys_puts(args);
                    sys_puts("\n");
                } else {
                    sys_puts("[edit] error: failed to write file!\n");
                }
                sys_exit();
            } 
            else if (str_equals(edit_line, ".cancel")) {
                sys_puts("\n[edit] cancelled.\n");
                sys_exit();
            } 
            else {
                append_line(edit_line);
                line_count++;
                edit_line_len = 0;
                sys_putchar('\n');
                print_number(line_count + 1);
                sys_puts(": ");
            }
        } 
        else if (c == '\b') {
            if (edit_line_len > 0) {
                edit_line_len--;
                sys_putchar('\b');
            }
        } 
        else if (edit_line_len < (int)LINE_MAX - 1) {
            edit_line[edit_line_len++] = c;
            sys_putchar(c);
        }
    }
}
