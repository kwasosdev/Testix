// binutils/kernlang.cpp

#include <syscall.hpp>
#include <kernlang.hpp>

extern "C" __attribute__((section(".text._start"))) void _start(const char* args) {
    if (!args || args == 0) {
        sys_puts("Usage: kernlang <script.kl>\n");
        sys_exit();
    }
    constexpr size_t SCRIPT_SIZE = 4096;
    static char file_script[SCRIPT_SIZE];
    int bytes = sys_read_file(args, file_script, sizeof(file_script) - 1);
    if (bytes < 0) {
        sys_puts("kernlang: script not found: ");
        sys_puts(args);
        sys_puts("\n");
    } else {
        file_script[bytes] = 0;
        kernlang::reset();
        kernlang::execute(file_script);
    }
    sys_exit();
}
