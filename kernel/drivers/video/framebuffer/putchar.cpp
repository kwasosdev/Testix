// kernel/drivers/video/framebuffer/putchar.cpp

#include "fb_internal.hpp"
#include <font8x8.hpp>

namespace fb {
    void putchar(char c, uint32_t x, uint32_t y, uint32_t color, uint32_t scale) {
        if (!buffer || x >= width || y >= height) return;
        const uint8_t* glyph = font::get_glyph(c);
        if (!glyph) return;
        for (int row = 0; row < 8; row++) {
            for (int col = 0; col < 8; col++) {
                if (glyph[row] & (1 << (7 - col))) {
                    fill_rect(x + col * scale, y + row * scale, scale, scale, color);
                }
            }
        }
    }
}
