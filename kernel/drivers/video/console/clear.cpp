// kernel/drivers/video/console/clear.cpp

#include "console_internal.hpp"

namespace console {
    void clear() {
        for (int l = 0; l < SCROLLBACK_LINES; l++) {
            for (int c = 0; c < MAX_COLS; c++) {
                buffer[l][c] = {' ', 0xFFFFFF};
            }
        }
        cursor_col = 0;
        cursor_line = 0;
        total_lines = 1;
        view_offset = 0;
        render_all();
    }
}
