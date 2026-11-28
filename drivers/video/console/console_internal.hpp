// drivers/video/console/console_internal.hpp

#pragma once
#include <console.hpp>

namespace console {
    constexpr uint32_t font_width = 8;
    constexpr uint32_t font_height = 8;

    extern BootInfo binfo_copy;
    extern BootInfo* binfo;
    extern uint32_t cur_x;
    extern uint32_t cur_y;
    extern uint32_t margin_x;
    extern uint32_t margin_y;
    extern uint32_t line_gap;
    extern uint32_t color;
    extern uint32_t scale;

    void newline();
}
