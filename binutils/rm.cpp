// binutils/rm.cpp

#include <syscall.hpp>
#include <string.hpp>

extern "C" __attribute__((section(".text._start"))) void _start(const char* args) {
    if (!args || args == 0) {
        sys_puts("Usage: rm <filename>\n");
        sys_exit();
    }
    if (sys_remove_file(args)) {
        sys_puts("Removed ");
        sys_puts(args);
        sys_puts("\n");
    } else {
        sys_puts("rm: file not found: ");
        sys_puts(args);
        sys_puts("\n");
    }
    sys_exit();
}
