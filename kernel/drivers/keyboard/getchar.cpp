// drivers/keyboard/getchar.cpp
#include "keyboard_internal.hpp"

namespace keyboard {
    char getchar() {
        if (buf_head == buf_tail) return 0;
        char c = buffer[buf_tail];
        buf_tail = (buf_tail + 1) % BUFFER_SIZE;
        return c;
    }
}