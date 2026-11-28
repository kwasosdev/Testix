// drivers/video/vga/init.cpp

#include "vga_internal.hpp"

namespace vga {
    void init() {
        load_custom_font();
        clear();

        outb(0x3D4, 0x0A);
        outb(0x3D5, 0x20);
    }
}