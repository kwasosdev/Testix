// binutils/man.cpp

#include <syscall.hpp>
#include <string.hpp>

constexpr size_t PATH_MAX = 64;
constexpr size_t PAGE_MAX = 4096;

extern "C" __attribute__((section(".text._start"))) void _start(const char* args) {
    // 1. Вызов без аргументов: показываем список всех доступных man-страниц
    if (!args || args[0] == 0) {
        sys_puts("Available manual pages (use 'man <command>'):\n  ");
        char fname[PATH_MAX];
        size_t fsize = 0;
        size_t idx = 0;
        bool first = true;

        while (sys_readdir(idx, fname, &fsize)) {
            if (str_starts_with(fname, "etc/man/man-")) {
                if (!first) sys_puts(", ");
                sys_set_color(0x55AAFF);
                sys_puts(fname + 12); // Пропускаем "etc/man/man-"
                sys_set_color(0xFFFFFF);
                first = false;
            }
            idx++;
        }
        if (first) sys_puts("(none)");
        sys_puts("\n");
        sys_exit();
    }

    // 2. Формируем путь "etc/man/man-<args>"
    const char* prefix = "etc/man/man-";
    char path[PATH_MAX];
    int p = 0;
    while (prefix[p]) {
        path[p] = prefix[p];
        p++;
    }
    int a = 0;
    while (args[a] && args[a] != ' ' && p < (int)PATH_MAX - 1) {
        path[p++] = args[a++];
    }
    path[p] = 0;

    static char page_buf[PAGE_MAX];
    int bytes = sys_read_file(path, page_buf, sizeof(page_buf) - 1);
    if (bytes < 0) {
        sys_puts("No manual entry for ");
        sys_puts(args);
        sys_puts("\n");
    } else {
        page_buf[bytes] = 0;
        sys_set_color(0x55AAFF);
        sys_puts("=== TESTIX MANUAL: ");
        sys_puts(args);
        sys_puts(" ===\n");
        sys_set_color(0xFFFFFF);

        sys_puts(page_buf);
        if (bytes > 0 && page_buf[bytes - 1] != '\n') sys_puts("\n");
    }

    sys_exit();
}
