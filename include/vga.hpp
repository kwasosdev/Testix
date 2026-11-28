// include/vga.hpp

#pragma once
#include <stdint.h>
#include <stddef.h>

namespace vga {
    void init();
    void clear();
    void putchar(char c);
    void puts(const char* s);
    
    // Новая функция
    void load_custom_font(); 
}