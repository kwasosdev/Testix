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

static inline char sys_getchar() {
    uint64_t ret;
    asm volatile("int $0x80" : "=a"(ret) : "a"(4ULL) : "memory");
    return (char)(ret & 0xFF);
}

static inline void sys_clear() {
    asm volatile("int $0x80" :: "a"(5ULL) : "memory");
}

static inline bool sys_spawn(uint64_t prog_id) {
    uint64_t ret;
    asm volatile("int $0x80" : "=a"(ret) : "a"(6ULL), "D"(prog_id) : "memory");
    return ret != 0;
}

static inline void sys_delay(uint64_t iterations) {
    for (volatile uint64_t i = 0; i < iterations; i = i + 1) {
        asm volatile("nop");
    }
}

static inline void sys_panic() {
    asm volatile("int $0x80" :: "a"(7ULL) : "memory");
    for (;;) asm volatile("nop");
}

static inline int sys_active_tasks() {
    uint64_t ret;
    asm volatile("int $0x80" : "=a"(ret) : "a"(8ULL) : "memory");
    return (int)ret;
}
