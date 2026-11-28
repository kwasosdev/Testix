// drivers/keyboard/init.cpp

#include "keyboard_internal.hpp"

namespace keyboard {
    void init() {
        shift_pressed = false;
        buf_head = 0;
        buf_tail = 0;
    }
}
