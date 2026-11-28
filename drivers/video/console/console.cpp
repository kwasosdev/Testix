// drivers/video/console/console.cpp

#include <console.hpp>

namespace console {
    static BootInfo binfo_copy;
    static BootInfo* binfo = nullptr;
    static uint32_t cur_x = 20;
    static uint32_t cur_y = 20;
    static uint32_t color = 0xFFFFFF;
    static uint32_t scale = 2;
    static uint32_t font_width = 8;
    static uint32_t font_height = 8;

    void init(BootInfo* info) {
        if (!info) return;
        binfo_copy = *info; // Копируем структуру, а не указатель!
        binfo = &binfo_copy;
        cur_x = 20;
        cur_y = 20;
        scale = 2;
    }

    void clear() {
        if (!binfo) return;
        fb::fill_rect(0, 0, binfo->width, binfo->height, 0x111111);
        cur_x = 20;
        cur_y = 20;
    }

    void putchar(char c) {
        if (!binfo) return;

        if (c == '\n') {
            cur_x = 20;
            cur_y += (font_height * scale) + 8;
        } 
        else if (c == '\b') {
            if (cur_x > 20) {
                cur_x -= (font_width * scale);
                fb::fill_rect(cur_x, cur_y, font_width * scale, font_height * scale, 0x111111);
            }
        } 
        else {
            if (cur_x + (font_width * scale) > binfo->width - 20) {
                cur_x = 20;
                cur_y += (font_height * scale) + 8;
            }
            char str[2] = {c, '\0'};
            fb::puts(str, cur_x, cur_y, color, scale);
            cur_x += (font_width * scale);
        }

        if (cur_y >= binfo->height - 20) {
            clear();
        }
    }

    void puts(const char* s) {
        while (*s) putchar(*s++);
    }
}