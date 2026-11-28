// drivers/video/framebuffer/framebuffer.cpp

#include "fb_internal.hpp"

namespace fb {
    uint8_t* buffer = nullptr;
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t pitch = 0;
    uint32_t bpp = 32;

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
}
