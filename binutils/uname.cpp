// binutils/uname.cpp

#include <syscall.hpp>

extern "C" __attribute__((section(".text._start"))) void _start(const char* args) {
    (void)args;
    sys_puts("Testix OS v0.1 (x86_64 Modular Userspace)\n");
    sys_exit();
}
