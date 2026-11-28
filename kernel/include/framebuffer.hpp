// include/framebuffer.hpp

#pragma once
#include <stdint.h>

// Эта структура должна быть строго такой
struct BootInfo {
    uint64_t framebuffer_base;
    uint32_t width;
    uint32_t height;
    uint32_t pixels_per_scanline;
    uint32_t bpp;
};

namespace fb {
    void init(BootInfo* info);
    bool is_initialized();
    
    // Геттеры для получения параметров экрана
    void* get_base();
    uint32_t get_width();
    uint32_t get_height();
    uint32_t get_bpp();

    void put_pixel(uint32_t x, uint32_t y, uint32_t color);
    void fill_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color);
    void putchar(char c, uint32_t x, uint32_t y, uint32_t color, uint32_t scale = 2);
    void puts(const char* str, uint32_t x, uint32_t y, uint32_t color, uint32_t scale = 2);
    void scroll_up(uint32_t pixels, uint32_t bg_color);
}