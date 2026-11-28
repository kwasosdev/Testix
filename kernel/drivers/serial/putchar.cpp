// drivers/serial/putchar.cpp

#include "serial_internal.hpp"

namespace serial {
    void putchar(char c) {
        int timeout = 100000;
        while ((inb(COM1_PORT + 5) & 0x20) == 0 && timeout > 0) {
            timeout--;
        }
        if (timeout > 0) {
            outb(COM1_PORT, c);
        }
    }
}
