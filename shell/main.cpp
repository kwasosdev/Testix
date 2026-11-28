// shell/main.cpp

#include <syscall.hpp>
#include <string.hpp>

constexpr size_t CMD_MAX  = 256;
constexpr size_t NAME_MAX = 64;

static char cmd_buffer[CMD_MAX];
static int cmd_len = 0;

static void print_prompt() {
    char cwd[NAME_MAX];
    sys_getcwd(cwd, sizeof(cwd));
    sys_set_color(0x55FF55);
    sys_puts("Testix");
    sys_set_color(0xFFFFFF);
    sys_puts(":");
    sys_set_color(0x55AAFF);
    sys_puts(cwd);
    sys_set_color(0xFFFFFF);
    sys_puts("> ");
}

static void execute_command() {
    if (cmd_len == 0) return;
    sys_puts("\n");

    char cmd_name[NAME_MAX];
    int i = 0;
    while (cmd_buffer[i] && cmd_buffer[i] != ' ' && i < (int)NAME_MAX - 1) {
        cmd_name[i] = cmd_buffer[i];
        i++;
    }
    cmd_name[i] = 0;

    const char* args = "";
    if (cmd_buffer[i] == ' ') {
        while (cmd_buffer[i] == ' ') i++;
        args = cmd_buffer + i;
    }

    if (str_equals(cmd_name, "help")) {
        sys_puts("Built-in shell commands:\n");
        sys_puts("  help     - Show this message\n");
        sys_puts("  cd <dir> - Change current directory\n");
        sys_puts("  clear    - Clear screen\n");
        sys_puts("  ring3    - Spawn concurrent tasks prog_a & prog_b\n");
        sys_puts("  exit     - Exit current shell\n");
        sys_puts("External utilities (in bin/):\n  ");

        char fname[NAME_MAX];
        size_t fsize = 0;
        size_t idx = 0;
        bool first = true;
        size_t col = 2;

        while (sys_readdir(idx, fname, &fsize)) {
            if (str_starts_with(fname, "bin/")) {
                const char* prog = fname + 4;
                if (!str_equals(prog, "init") && !str_equals(prog, "shell")) {
                    size_t plen = str_len(prog) + (first ? 0 : 2);
                    if (col + plen > 58) {
                        sys_puts("\n  ");
                        col = 2;
                        first = true;
                    }
                    if (!first) sys_puts(", ");
                    sys_set_color(0x55FF55);
                    sys_puts(prog);
                    sys_set_color(0xFFFFFF);
                    col += plen;
                    first = false;
                }
            }
            idx++;
        }
        sys_puts("\n");
    }
    else if (str_equals(cmd_name, "cd")) {
        if (!sys_chdir(args)) {
            sys_puts("cd: no such directory: ");
            sys_puts(args);
            sys_puts("\n");
        }
    }
    else if (str_equals(cmd_name, "clear")) {
        sys_clear();
    }
    else if (str_equals(cmd_name, "exit")) {
        sys_puts("[shell] exiting...\n");
        sys_exit();
    }
    else if (str_equals(cmd_name, "ring3")) {
        sys_puts("[shell] spawning user tasks A and B...\n");
        int before = sys_active_tasks();
        sys_spawn("/bin/prog_a");
        sys_spawn("/bin/prog_b");
        while (sys_active_tasks() > before) sys_yield();
        sys_puts("\n[shell] all tasks finished.\n");
    }
    else {
        const char* target_bin = str_equals(cmd_name, "run") ? "kernlang" : cmd_name;
        char bin_path[NAME_MAX];
        const char* prefix = "/bin/";
        for (int p = 0; p < 5; p++) bin_path[p] = prefix[p];
        int k = 0;
        while (target_bin[k] && k < (int)NAME_MAX - 6) {
            bin_path[5 + k] = target_bin[k];
            k++;
        }
        bin_path[5 + k] = 0;

        int before = sys_active_tasks();
        if (sys_spawn(bin_path, args)) {
            while (sys_active_tasks() > before) {
                sys_yield();
            }
        } else {
            sys_puts("Unknown command: ");
            sys_puts(cmd_name);
            sys_puts("\nType 'help' for commands.\n");
        }
    }

    cmd_len = 0;
    *cmd_buffer = 0;
    print_prompt();
}

static void handle_char(char c) {
    if (c == '\n') {
        cmd_buffer[cmd_len] = 0;
        execute_command();
    } else if (c == '\b') {
        if (cmd_len > 0) {
            cmd_len--;
            sys_putchar('\b');
        }
    } else if (cmd_len < (int)CMD_MAX - 1) {
        cmd_buffer[cmd_len++] = c;
        sys_putchar(c);
    }
}

extern "C" __attribute__((section(".text._start"))) void _start(const char* args) {
    (void)args;
    print_prompt();
    for (;;) {
        char c = sys_getchar();
        if (c != 0) handle_char(c);
        else sys_yield();
    }
}
