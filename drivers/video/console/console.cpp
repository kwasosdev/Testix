#include <console.hpp>

namespace console {
    static BootInfo binfo_copy;
    static BootInfo* binfo = nullptr;
    static uint32_t cur_x = 0;
    static uint32_t cur_y = 0;
    static uint32_t margin_x = 16;
    static uint32_t margin_y = 16;
    static uint32_t line_gap = 6;
    static uint32_t color = 0xFFFFFF;
    static uint32_t scale = 2;
    static constexpr uint32_t font_width = 8;
    static constexpr uint32_t font_height = 8;

    static void newline() {
        cur_x = margin_x;
        cur_y += (font_height * scale) + line_gap;
        if (cur_y + (font_height * scale) + margin_y > binfo->height) {
            clear();
        }
    }

    void init(BootInfo* info) {
        if (!info) return;
        binfo_copy = *info;
        binfo = &binfo_copy;

        // Адаптивный масштаб: 1x для экранов < 1024px по ширине, 2x для HD/FullHD/2K
        scale = (binfo->width >= 1024) ? 2 : 1;
        margin_x = (binfo->width >= 1024) ? 20 : 10;
        margin_y = (binfo->height >= 768) ? 20 : 10;
        line_gap = (scale == 2) ? 8 : 4;

        cur_x = margin_x;
        cur_y = margin_y;
    }

    void clear() {
        if (!binfo) return;
        fb::fill_rect(0, 0, binfo->width, binfo->height, 0x111111);
        cur_x = margin_x;
        cur_y = margin_y;
    }

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

    void puts(const char* s) {
        while (*s) putchar(*s++);
    }
}
