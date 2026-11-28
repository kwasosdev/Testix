// user/prog_b.cpp

#include "syscall.hpp"

__attribute__((section(".text._start")))
extern "C" void _start() {
    for (int i = 0; i < 15; i++) {
        sys_putchar('b');
        sys_delay(3000000);
        sys_yield();
    }
    sys_exit();
}