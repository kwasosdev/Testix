// drivers/keyboard/keyboard_internal.hpp

#pragma once
#include <keyboard.hpp>
#include <stdint.h>

namespace keyboard {
    constexpr uint16_t DATA_PORT = 0x60;
    constexpr int BUFFER_SIZE = 256;

    extern const char keymap[128];
    extern const char keymap_shift[128];

    extern bool shift_pressed;
    extern char buffer[BUFFER_SIZE];
    extern int buf_head;
    extern int buf_tail;

    static inline uint8_t inb(uint16_t port) {
        uint8_t ret;
        asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
        return ret;
    }
}
