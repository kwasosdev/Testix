// binutils/cat.cpp

#include <syscall.hpp>
#include <string.hpp>

extern "C" __attribute__((section(".text._start"))) void _start(const char* args) {
    if (!args || args == 0) {
        sys_puts("Usage: cat <filename>\n");
        sys_exit();
    }
    constexpr size_t BUF_SIZE = 4096;
    static char file_buf[BUF_SIZE];
    int bytes = sys_read_file(args, file_buf, sizeof(file_buf) - 1);
    if (bytes < 0) {
        sys_puts("cat: file not found: ");
        sys_puts(args);
        sys_puts("\n");
    } else {
        file_buf[bytes] = 0;
        sys_puts(file_buf);
        if (bytes > 0 && file_buf[bytes - 1] != '\n') sys_puts("\n");
    }
    sys_exit();
}
