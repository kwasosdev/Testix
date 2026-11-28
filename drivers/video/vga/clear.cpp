// drivers/video/vga/clear.cpp

#include "vga_internal.hpp"

namespace vga {
    void clear() {
        for (size_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; ++i) {
            VGA_BUFFER[i] = (color << 8) | ' ';
        }
        row = 0;
        col = 0;
        update_cursor();
    }
}
