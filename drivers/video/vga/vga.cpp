// drivers/video/vga/vga.cpp

#include "vga_internal.hpp"

namespace vga {
    uint8_t color = 0x0F;
    size_t row = 0;
    size_t col = 0;

    void update_cursor() {
        uint16_t pos = row * VGA_WIDTH + col;
        outb(0x3D4, 0x0F);
        outb(0x3D5, (uint8_t)(pos & 0xFF));
        outb(0x3D4, 0x0E);
        outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
    }

    void write_seq(uint8_t index, uint8_t data) { outb(0x3C4, index); outb(0x3C5, data); }
    uint8_t read_seq(uint8_t index) { outb(0x3C4, index); return inb(0x3C5); }
    void write_gc(uint8_t index, uint8_t data) { outb(0x3CE, index); outb(0x3CF, data); }
    uint8_t read_gc(uint8_t index) { outb(0x3CE, index); return inb(0x3CF); }
}
