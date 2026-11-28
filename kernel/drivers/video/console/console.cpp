// kernel/drivers/video/console/console.cpp

#include "console_internal.hpp"

namespace console {
    BootInfo binfo_copy;
    BootInfo* binfo = nullptr;
    uint32_t margin_x = 16;
    uint32_t margin_y = 16;
    uint32_t line_gap = 6;
    uint32_t current_color = 0xFFFFFF;
    uint32_t scale = 2;

    int cols = 80;
    int visible_rows = 24;

    Cell buffer[SCROLLBACK_LINES][MAX_COLS];
    int cursor_col = 0;
    int cursor_line = 0;
    int total_lines = 1;
    int view_offset = 0;
    static bool cursor_visible = true;
    static uint32_t blink_ticks = 0;

    void set_color(uint32_t new_color) {
        current_color = new_color;
    }

    void render_line(int screen_row, int buf_line) {
        if (!binfo || screen_row < 0 || screen_row >= visible_rows) return;
        uint32_t char_w = font_width * scale;
        uint32_t char_h = font_height * scale;
        uint32_t y = margin_y + (uint32_t)screen_row * (char_h + line_gap);

        fb::fill_rect(0, y, binfo->width, char_h + line_gap, 0x111111);
        if (buf_line < 0 || buf_line >= visible_rows) return;

        for (int c = 0; c < cols; c++) {
            char ch = buffer[buf_line][c].ch;
            if (ch != 0 && ch != ' ') {
                fb::putchar(ch, margin_x + (uint32_t)c * char_w, y, buffer[buf_line][c].color, scale);
            }
        }
    }

    void render_all() {
        if (!binfo) return;
        fb::fill_rect(0, 0, binfo->width, binfo->height, 0x111111);
        for (int r = 0; r < visible_rows; r++) {
            render_line(r, r);
        }
    }

    void scroll_view(int delta_lines) {
        (void)delta_lines;
    }

    void newline() {
        draw_cursor(false); // Стираем курсор на старой строке
        cursor_col = 0;
        cursor_line++;

        if (cursor_line >= visible_rows) {
            for (int r = 0; r < visible_rows - 1; r++) {
                for (int c = 0; c < cols; c++) {
                    buffer[r][c] = buffer[r + 1][c];
                }
            }
            for (int c = 0; c < cols; c++) {
                buffer[visible_rows - 1][c] = {' ', 0xFFFFFF};
            }
            cursor_line = visible_rows - 1;
            render_all();
        }
        cursor_visible = true;
        blink_ticks = 0;
        draw_cursor(true); // Рисуем курсор на новой строке
    }

    void draw_cursor(bool show) {
        if (!binfo || cursor_line < 0 || cursor_line >= visible_rows) return;
        if (cursor_col < 0 || cursor_col >= cols) return;

        uint32_t char_w = font_width * scale;
        uint32_t char_h = font_height * scale;
        uint32_t px = margin_x + (uint32_t)cursor_col * char_w;
        uint32_t py = margin_y + (uint32_t)cursor_line * (char_h + line_gap);

        // Сначала восстанавливаем ячейку под курсором (на случай, если там пробел или символ)
        fb::fill_rect(px, py, char_w, char_h + 2, 0x111111);
        char ch = buffer[cursor_line][cursor_col].ch;
        if (ch != 0 && ch != ' ') {
            fb::putchar(ch, px, py, buffer[cursor_line][cursor_col].color, scale);
        }

        // Рисуем нижнюю черту курсора толщиной 2*scale пикселей
        if (show) {
            uint32_t bar_h = 2 * scale;
            fb::fill_rect(px, py + char_h - bar_h + 2, char_w, bar_h, current_color);
        }
    }

    void tick() {
        if (!binfo) return;
        blink_ticks++;
        if (blink_ticks >= 50) { // 50 тиков при 100 Гц = 0.5 секунды
            blink_ticks = 0;
            cursor_visible = !cursor_visible;
            draw_cursor(cursor_visible);
        }
    }
}
