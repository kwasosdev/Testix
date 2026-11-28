// drivers/video/console/init.cpp

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

        cur_x = margin_x;
        cur_y = margin_y;
    }
}
