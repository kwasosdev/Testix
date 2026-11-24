#pragma once
#include <stdint.h>

namespace serial {
    void init();
    void putchar(char c);
    void puts(const char* s);
    void put_hex(uint32_t val);
    void put_hex64(uint64_t val);
}