// drivers/serial/init.cpp

#include "serial_internal.hpp"

namespace serial {
    void init() {
        outb(COM1_PORT + 1, 0x00);
        outb(COM1_PORT + 3, 0x80);
        outb(COM1_PORT + 0, 0x03);
        outb(COM1_PORT + 1, 0x00);
        outb(COM1_PORT + 3, 0x03);
        outb(COM1_PORT + 2, 0xC7);
        outb(COM1_PORT + 4, 0x0B);
    }
}
