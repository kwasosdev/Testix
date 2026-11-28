// drivers/video/framebuffer/fill_rect.cpp

#include "fb_internal.hpp"

namespace fb {
    void fill_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color) {
        if (!buffer || x >= width || y >= height) return;

        if (w > width - x) w = width - x;
        if (h > height - y) h = height - y;

        if (bpp == 32) {
            uint32_t* ptr = (uint32_t*)buffer;
            for (uint32_t j = 0; j < h; ++j) {
                uint32_t* row = ptr + (y + j) * pitch + x;
                for (uint32_t i = 0; i < w; ++i) {
                    row[i] = color;
                }
            }
            return;
        }

        for (uint32_t j = 0; j < h; ++j) {
            for (uint32_t i = 0; i < w; ++i) {
                put_pixel(x + i, y + j, color);
            }
        }
    }
}
