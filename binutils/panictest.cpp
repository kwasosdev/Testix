// binutils/panictest.cpp

#include <syscall.hpp>

extern "C" __attribute__((section(".text._start"))) void _start(const char* args) {
    (void)args;
    sys_panic();
}
