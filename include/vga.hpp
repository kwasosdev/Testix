// include/vga.hpp

#pragma once
#include <stdint.h>
#include <stddef.h>

namespace vga {
    void init();
    void clear();
    void putchar(char c);
    void puts(const char* s);

    void write_seq(uint8_t index, uint8_t data);
    uint8_t read_seq(uint8_t index);
    void write_gc(uint8_t index, uint8_t data);
    uint8_t read_gc(uint8_t index);
    void load_custom_font();
}