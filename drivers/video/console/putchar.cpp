// drivers/video/console/putchar.cpp

#include "console_internal.hpp"

namespace console {
    void putchar(char c) {
        if (!binfo) return;

        uint32_t char_w = font_width * scale;
        uint32_t char_h = font_height * scale;

        if (c == '\n') {
            newline();
        } 
        else if (c == '\r') {
            cur_x = margin_x;
        }
        else if (c == '\b') {
            if (cur_x >= margin_x + char_w) {
                cur_x -= char_w;
                fb::fill_rect(cur_x, cur_y, char_w, char_h, 0x111111);
            }
        } 
        else {
            if (cur_x + char_w + margin_x > binfo->width) {
                newline();
            }
            char str[2] = {c, '\0'};
            fb::puts(str, cur_x, cur_y, color, scale);
            cur_x += char_w;
        }
    }
}
