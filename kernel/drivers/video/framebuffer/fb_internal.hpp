// kernel/drivers/video/framebuffer/fb_internal.hpp
#pragma once
#include <framebuffer.hpp>

namespace fb {
    extern uint8_t* buffer;
    extern uint32_t width;
    extern uint32_t height;
    extern uint32_t pitch;
    extern uint32_t bpp;
}
