// binutils/echo.cpp

#include <syscall.hpp>
#include <string.hpp>

constexpr size_t OUT_MAX  = 512;
constexpr size_t NAME_MAX = 64;

extern "C" __attribute__((section(".text._start"))) void _start(const char* args) {
    if (!args || args[0] == 0) {
        sys_puts("\n");
        sys_exit();
    }

    // Ищем оператор перенаправления '>'
    int redirect_pos = -1;
    for (int i = 0; args[i]; i++) {
        if (args[i] == '>') {
            redirect_pos = i;
            break;
        }
    }

    // Обычный вывод на экран
    if (redirect_pos == -1) {
        sys_puts(args);
        sys_puts("\n");
        sys_exit();
    }

    // Извлекаем текст до '>' (обрезая пробелы в конце)
    char text[OUT_MAX];
    int tlen = redirect_pos;
    while (tlen > 0 && args[tlen - 1] == ' ') tlen--;
    if (tlen >= (int)OUT_MAX - 2) tlen = (int)OUT_MAX - 2;
    for (int i = 0; i < tlen; i++) text[i] = args[i];
    text[tlen++] = '\n';
    text[tlen] = 0;

    // Извлекаем имя файла после '>' (пропуская пробелы)
    int fpos = redirect_pos + 1;
    while (args[fpos] == ' ') fpos++;

    char fname[NAME_MAX];
    int flen = 0;
    while (args[fpos + flen] && args[fpos + flen] != ' ' && flen < (int)NAME_MAX - 1) {
        fname[flen] = args[fpos + flen];
        flen++;
    }
    fname[flen] = 0;

    if (flen == 0) {
        sys_puts("echo: missing filename after '>'\n");
        sys_exit();
    }

    if (!sys_write_file(fname, text, (size_t)tlen)) {
        sys_puts("echo: failed to write ");
        sys_puts(fname);
        sys_puts("\n");
    }

    sys_exit();
}
