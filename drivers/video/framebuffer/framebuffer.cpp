#include <framebuffer.hpp>
#include <font8x8.hpp>

namespace fb {
    static uint8_t* buffer = nullptr;
    static uint32_t width = 0;
    static uint32_t height = 0;
    static uint32_t pitch = 0;
    static uint32_t bpp = 32;

    void init(BootInfo* info) {
        if (!info) return;
        buffer = (uint8_t*)info->framebuffer_base;
        width = info->width;
        height = info->height;
        pitch = info->pixels_per_scanline;
        bpp = info->bpp;
    }

    bool is_initialized() {
        return buffer != nullptr;
    }

    void* get_base() { return buffer; }
    uint32_t get_width() { return width; }
    uint32_t get_height() { return height; }
    uint32_t get_bpp() { return bpp; }

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

    void fill_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color) {
        if (!buffer || x >= width || y >= height) return;

        // Автоматическое обрезание по реальным границам монитора
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
