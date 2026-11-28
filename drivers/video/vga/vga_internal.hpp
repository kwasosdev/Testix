// drivers/video/vga/vga_internal.hpp

#pragma once
#include <vga.hpp>
#include <font8x8.hpp>

namespace vga {
    constexpr size_t VGA_WIDTH  = 80;
    constexpr size_t VGA_HEIGHT = 50;
    inline volatile uint16_t* const VGA_BUFFER =
        reinterpret_cast<volatile uint16_t*>(0xB8000);

    // Общее состояние (определяется один раз в vga.cpp)
    extern uint8_t color;
    extern size_t row;
    extern size_t col;

    // Вспомогательные функции портов (inline, чтобы работали в любом .cpp)
    static inline void outb(uint16_t port, uint8_t val) {
        asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
    }

    static inline uint8_t inb(uint16_t port) {
        uint8_t ret;
        asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
        return ret;
    }

    void update_cursor();
}
