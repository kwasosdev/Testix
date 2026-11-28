// user/prog_a.cpp

#include "syscall.hpp"

extern "C" __attribute__((section(".text._start"))) void _start() {
    sys_puts("Hello from Ring 3!\n");
    for (int i = 0; i < 15; i++) {
        sys_putchar('a');
        sys_delay(3000000);
        sys_yield();
    }
    sys_exit();
}