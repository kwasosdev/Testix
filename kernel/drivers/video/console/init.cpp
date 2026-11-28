// kernel/drivers/video/console/init.cpp
#include "console_internal.hpp"

namespace console {
    void init(BootInfo* info) {
        if (!info) return;
        binfo_copy = *info;
        binfo = &binfo_copy;

        scale = (binfo->width >= 1024) ? 2 : 1;
        margin_x = (binfo->width >= 1024) ? 20 : 10;
        margin_y = (binfo->height >= 768) ? 20 : 10;
        line_gap = (scale == 2) ? 8 : 4;

        uint32_t char_w = font_width * scale;
        uint32_t char_h = font_height * scale;
        uint32_t row_step = char_h + line_gap;

        cols = (int)((binfo->width - margin_x * 2) / char_w);
        if (cols > MAX_COLS) cols = MAX_COLS;

        visible_rows = (int)((binfo->height - margin_y * 2) / row_step);
        if (visible_rows < 1) visible_rows = 1;
        clear();
    }
}
