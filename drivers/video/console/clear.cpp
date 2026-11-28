// drivers/video/console/clear.cpp

#include "console_internal.hpp"

namespace console {
    void clear() {
        if (!binfo) return;
        fb::fill_rect(0, 0, binfo->width, binfo->height, 0x111111);
        cur_x = margin_x;
        cur_y = margin_y;
    }
}
