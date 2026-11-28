// drivers/video/framebuffer/puts.cpp

#include "fb_internal.hpp"

namespace fb {
    void puts(const char* str, uint32_t x, uint32_t y, uint32_t color, uint32_t scale) {
        if (!buffer) return;
        uint32_t start_x = x;
        while (*str) {
            if (*str == '\n') {
                y += 8 * scale + 4;
                x = start_x;
            } else {
                if (x + 8 * scale > width) {
                    x = start_x;
                    y += 8 * scale + 4;
                }
                if (y + 8 * scale > height) break;
                putchar(*str, x, y, color, scale);
                x += 8 * scale;
            }
            str++;
        }
    }
}
