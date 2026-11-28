// libsys/include/syscall.hpp

#pragma once
#include <stdint.h>
#include <stddef.h>

static inline void sys_putchar(char c) {
    asm volatile("int $0x80" :: "a"(1ULL), "D"((uint64_t)(uint8_t)c) : "memory");
}

static inline void sys_puts(const char* s) {
    while (*s) sys_putchar(*s++);
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

static inline bool sys_spawn(const char* path, const char* args = "") {
    uint64_t ret;
    asm volatile("int $0x80" : "=a"(ret) : "a"(6ULL), "D"(path), "S"(args) : "memory");
    return ret != 0;
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

static inline bool sys_readdir(size_t index, char* out_name, size_t* out_size) {
    uint64_t ret;
    asm volatile("int $0x80" : "=a"(ret) : "a"(9ULL), "D"(index), "S"(out_name), "d"(out_size) : "memory");
    return ret != 0;
}

static inline int sys_read_file(const char* name, void* buf, size_t max_size) {
    int64_t ret;
    asm volatile("int $0x80" : "=a"(ret) : "a"(10ULL), "D"(name), "S"(buf), "d"(max_size) : "memory");
    return (int)ret;
}

static inline bool sys_write_file(const char* name, const void* data, size_t size) {
    uint64_t ret;
    asm volatile("int $0x80" : "=a"(ret) : "a"(11ULL), "D"(name), "S"(data), "d"(size) : "memory");
    return ret != 0;
}

static inline bool sys_remove_file(const char* name) {
    uint64_t ret;
    asm volatile("int $0x80" : "=a"(ret) : "a"(12ULL), "D"(name) : "memory");
    return ret != 0;
}

static inline void sys_set_color(uint32_t rgb) {
    asm volatile("int $0x80" :: "a"(13ULL), "D"((uint64_t)rgb) : "memory");
}

static inline void sys_delay(uint64_t iterations) {
    for (volatile uint64_t i = 0; i < iterations; i = i + 1) {
        asm volatile("nop");
    }
}

static inline bool sys_chdir(const char* path) {
    uint64_t ret;
    asm volatile("int $0x80" : "=a"(ret) : "a"(14ULL), "D"(path) : "memory");
    return ret != 0;
}

static inline void sys_getcwd(char* buf, size_t max_len) {
    asm volatile("int $0x80" :: "a"(15ULL), "D"(buf), "S"(max_len) : "memory");
}
