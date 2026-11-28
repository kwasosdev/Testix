// kernel/drivers/video/console/putchar.cpp

#include "console_internal.hpp"

namespace console {
    void putchar(char c) {
        if (!binfo) return;

        draw_cursor(false);

        if (c == '\n') {
            newline();
            return;
        } 
        else if (c == '\r') {
            cursor_col = 0;
        }
        else if (c == '\t') {
            int next_col = ((cursor_col / 4) + 1) * 4;
            if (next_col >= cols) {
                newline();
                return;
            }
            cursor_col = next_col;
        }
        else if (c == '\b') {
            if (cursor_col > 0) {
                cursor_col--;
                buffer[cursor_line][cursor_col] = {' ', current_color};
                render_line(cursor_line, cursor_line);
            }
        } 
        else if ((uint8_t)c >= 32) {
            if (cursor_col >= cols) {
                newline();
            }
            buffer[cursor_line][cursor_col] = {c, current_color};

            uint32_t char_w = font_width * scale;
            uint32_t char_h = font_height * scale;
            uint32_t px = margin_x + (uint32_t)cursor_col * char_w;
            uint32_t py = margin_y + (uint32_t)cursor_line * (char_h + line_gap);
            fb::putchar(c, px, py, current_color, scale);

            cursor_col++;
        }

        draw_cursor(true);
    }
}
