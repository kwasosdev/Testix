// drivers/serial/serial_internal.hpp

#pragma once
#include <serial.hpp>
#include <stdint.h>

namespace serial {
    constexpr uint16_t COM1_PORT = 0x3F8;

    static inline void outb(uint16_t port, uint8_t val) {
        asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
    }

    static inline uint8_t inb(uint16_t port) {
        uint8_t ret;
        asm volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
        return ret;
    }
}
