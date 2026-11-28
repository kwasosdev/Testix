// drivers/video/framebuffer/put_pixel.cpp

#include "fb_internal.hpp"

namespace fb {
    void put_pixel(uint32_t x, uint32_t y, uint32_t color) {
        if (!buffer || x >= width || y >= height) return;

        if (bpp == 32) {
            uint32_t* ptr = (uint32_t*)buffer;
            ptr[y * pitch + x] = color;
        } else if (bpp == 24) {
            uint32_t offset = (y * pitch + x) * 3;
            buffer[offset + 0] = (color >> 0) & 0xFF;
            buffer[offset + 1] = (color >> 8) & 0xFF;
            buffer[offset + 2] = (color >> 16) & 0xFF;
        } else if (bpp == 16 || bpp == 15) {
            uint8_t r = (color >> 16) & 0xFF;
            uint8_t g = (color >> 8) & 0xFF;
            uint8_t b = (color >> 0) & 0xFF;
            uint16_t pixel = ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3);
            uint16_t* ptr = (uint16_t*)buffer;
            ptr[y * pitch + x] = pixel;
        }
    }
}
