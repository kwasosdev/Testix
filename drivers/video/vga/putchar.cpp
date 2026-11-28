// drivers/video/vga/putchar.cpp

#include "vga_internal.hpp"

namespace vga {
    void putchar(char c) {
        if (c == '\n') { 
            col = 0; 
            ++row; 
        }
        else if (c == '\r') { 
            col = 0; 
        }
        else {
            VGA_BUFFER[row * VGA_WIDTH + col] = (color << 8) | static_cast<uint8_t>(c);
            if (++col >= VGA_WIDTH) { col = 0; ++row; }
        }
        
        if (row >= VGA_HEIGHT) row = 0; 
        update_cursor();
    }

    void puts(const char* s) {
        while (*s) putchar(*s++);
    }
}
