// kernel/drivers/video/console/console_internal.hpp

#pragma once
#include <console.hpp>

namespace console {
    constexpr uint32_t font_width  = 8;
    constexpr uint32_t font_height = 8;
    constexpr int MAX_COLS         = 160;
    constexpr int SCROLLBACK_LINES = 200;

    struct Cell {
        char ch;
        uint32_t color;
    };

    extern BootInfo binfo_copy;
    extern BootInfo* binfo;
    extern uint32_t margin_x;
    extern uint32_t margin_y;
    extern uint32_t line_gap;
    extern uint32_t current_color;
    extern uint32_t scale;

    extern int cols;
    extern int visible_rows;

    extern Cell buffer[SCROLLBACK_LINES][MAX_COLS];
    extern int cursor_col;
    extern int cursor_line;   // Текущая строка записи (0 .. SCROLLBACK_LINES-1)
    extern int total_lines;   // Сколько строк заполнено
    extern int view_offset;   // Смещение просмотра вверх от конца (0 = самый низ)

    void render_all();
    void render_line(int screen_row, int buf_line);
    void newline();
    void draw_cursor(bool show);
}
