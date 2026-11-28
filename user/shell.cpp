// user/shell.cpp
#include "syscall.hpp"
#include "kernlang.hpp"

static char cmd_buffer[256];
static int cmd_len = 0;

static char script_buf[2048];
static int script_len = 0;
static bool editor_mode = false;
static char edit_line[128];
static int edit_line_len = 0;
static int line_count = 0;

static bool str_equals(const char* a, const char* b) {
    while (*a && *b) { if (*a != *b) return false; a++; b++; }
    return *a == *b;
}

static void print_number(int n) {
    if (n == 0) { sys_putchar('0'); return; }
    char buf[16];
    int k = 0;
    while (n > 0) { buf[k++] = '0' + (n % 10); n /= 10; }
    while (k > 0) sys_putchar(buf[--k]);
}

static void script_append_line(const char* line) {
    while (*line && script_len < 2047) script_buf[script_len++] = *line++;
    if (script_len < 2047) script_buf[script_len++] = '\n';
    script_buf[script_len] = 0;
}

static void run_script() {
    if (script_len == 0) {
        sys_puts("[shell] no script. use 'edit' first.\n");
        return;
    }
    sys_puts("[shell] running script...\n");
    sys_puts("------------------------------\n");
    kernlang::reset();
    kernlang::execute(script_buf);
    sys_puts("------------------------------\n");
    sys_puts("[shell] done.\n");
}

static void execute_command() {
    if (cmd_len == 0) return;
    sys_puts("\n");

    if (str_equals(cmd_buffer, "help")) {
        sys_puts("Available commands (Ring 3 Shell):\n");
        sys_puts("  help      - Show this message\n");
        sys_puts("  clear     - Clear screen\n");
        sys_puts("  uname     - Show name and version of OS\n");
        sys_puts("  panictest - Test Kernel Panic\n");
        sys_puts("  edit      - Enter script editor\n");
        sys_puts("  list      - Show current script\n");
        sys_puts("  run       - Execute current script\n");
        sys_puts("  del       - Delete script\n");
        sys_puts("  ring3     - Spawn user tasks A and B\n");
    } 
    else if (str_equals(cmd_buffer, "clear")) {
        sys_clear();
    } 
    else if (str_equals(cmd_buffer, "uname")) {
        sys_puts("Testix OS v0.1 (Userspace Shell)\n");
    }
    else if (str_equals(cmd_buffer, "panictest")) {
        sys_panic();
    }
    else if (str_equals(cmd_buffer, "edit")) {
        editor_mode = true;
        script_len = 0;
        line_count = 0;
        edit_line_len = 0;
        cmd_len = 0;
        cmd_buffer[0] = 0;
        
        sys_puts("[editor] enter lines. '.save' to save, '.cancel' to exit.\n");
        sys_puts("1: ");
    }
    else if (str_equals(cmd_buffer, "list")) {
        if (script_len == 0) sys_puts("[shell] script is empty.\n");
        else { sys_puts("[shell] current script:\n"); sys_puts(script_buf); }
    }
    else if (str_equals(cmd_buffer, "run")) {
        run_script();
    }
    else if (str_equals(cmd_buffer, "del")) {
        script_len = 0;
        script_buf[0] = 0;
        sys_puts("[shell] script deleted.\n");
    }
    else if (str_equals(cmd_buffer, "ring3")) {
        sys_puts("[shell] spawning user tasks A and B...\n");
        sys_spawn(1);
        sys_spawn(2);
        while (sys_active_tasks() > 1) {
            sys_yield();
        }
        sys_puts("\n[shell] all tasks finished.\n");
    }
    else {
        sys_puts("Unknown command: ");
        sys_puts(cmd_buffer);
        sys_puts("\nType 'help' for commands.\n");
    }

    if (!editor_mode) {
        cmd_len = 0;
        cmd_buffer[0] = 0;
        sys_puts("Testix> ");
    }
}

static void handle_char(char c) {
    if (editor_mode) {
        if (c == '\n') {
            edit_line[edit_line_len] = 0;
            if (str_equals(edit_line, ".save")) {
                editor_mode = false;
                cmd_len = 0;
                cmd_buffer[0] = 0;
                sys_puts("\n[editor] saved ");
                print_number(line_count);
                sys_puts(" lines.\nTestix> ");
            } 
            else if (str_equals(edit_line, ".cancel")) {
                editor_mode = false;
                cmd_len = 0;
                cmd_buffer[0] = 0;
                sys_puts("\n[editor] cancelled.\nTestix> ");
            } 
            else {
                script_append_line(edit_line);
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
        else if (edit_line_len < 127) {
            edit_line[edit_line_len++] = c;
            sys_putchar(c);
        }
        return;
    }
    
    if (c == '\n') {
        cmd_buffer[cmd_len] = 0;
        execute_command();
    } 
    else if (c == '\b') {
        if (cmd_len > 0) {
            cmd_len--;
            sys_putchar('\b');
        }
    } 
    else if (cmd_len < 255) {
        cmd_buffer[cmd_len++] = c;
        sys_putchar(c);
    }
}

extern "C" __attribute__((section(".text._start"))) void _start() {
    sys_puts("Testix> ");
    for (;;) {
        char c = sys_getchar();
        if (c != 0) {
            handle_char(c);
        } else {
            sys_yield();
        }
    }
}

