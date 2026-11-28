// kernel/include/console.hpp
#pragma once
#include <framebuffer.hpp>
#include <stdint.h>

namespace console {
    void init(BootInfo* info);
    void clear();
    void putchar(char c);
    void puts(const char* s);
    void set_color(uint32_t new_color);
    void scroll_view(int delta_lines);
    void tick();
}
