// include/console.hpp

#pragma once
#include <framebuffer.hpp>

namespace console {
    void init(BootInfo* info);
    void putchar(char c);
    void puts(const char* s);
    void clear();
}