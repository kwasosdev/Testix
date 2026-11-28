// drivers/keyboard/handle_interrupt.cpp

#include "keyboard_internal.hpp"

namespace keyboard {
    void handle_interrupt() {
        uint8_t scancode = inb(DATA_PORT);

        if (scancode == 0x2A || scancode == 0x36) {
            shift_pressed = true;
            return;
        }
        if (scancode == 0xAA || scancode == 0xB6) {
            shift_pressed = false;
            return;
        }

        if (scancode < 128) {
            char c = shift_pressed ? keymap_shift[scancode] : keymap[scancode];
            if (c != 0) {
                buffer[buf_head] = c;
                buf_head = (buf_head + 1) % BUFFER_SIZE;
            }
        }
    }
}
