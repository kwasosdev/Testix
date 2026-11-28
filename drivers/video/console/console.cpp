// drivers/video/console/console.cpp

#include "console_internal.hpp"

namespace console {
    BootInfo binfo_copy;
    BootInfo* binfo = nullptr;
    uint32_t cur_x = 0;
    uint32_t cur_y = 0;
    uint32_t margin_x = 16;
    uint32_t margin_y = 16;
    uint32_t line_gap = 6;
    uint32_t color = 0xFFFFFF;
    uint32_t scale = 2;

    void newline() {
        cur_x = margin_x;
        cur_y += (font_height * scale) + line_gap;
        if (cur_y + (font_height * scale) + margin_y > binfo->height) {
            clear();
        }
    }
}
