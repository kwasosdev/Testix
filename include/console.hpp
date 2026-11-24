#pragma once
#include <stdint.h>
#include <framebuffer.hpp>

namespace console {
    void init(BootInfo* info);
    void putchar(char c);
    void puts(const char* s);
    void clear();
}