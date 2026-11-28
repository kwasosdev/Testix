// user/syscall.hpp
#pragma once
#include <stdint.h>

static inline void sys_putchar(char c) {
    asm volatile("int $0x80" :: "a"(1ULL), "D"((uint64_t)(uint8_t)c) : "memory");
}

static inline void sys_puts(const char* s) {
    while (*s) {
        sys_putchar(*s++);
    }
}

static inline void sys_yield() {
    asm volatile("int $0x80" :: "a"(2ULL) : "memory");
}

static inline void sys_exit() {
    asm volatile("int $0x80" :: "a"(3ULL) : "memory");
    for (;;) asm volatile("nop");
}

static inline void sys_delay(uint64_t iterations) {
    for (volatile uint64_t i = 0; i < iterations; i++) {
        asm volatile("nop");
    }
}