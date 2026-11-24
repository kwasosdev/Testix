#include <serial.hpp>
#include <stdint.h>

namespace serial {
    static inline void outb(uint16_t port, uint8_t val) {
        asm volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) );
    }
    static inline uint8_t inb(uint16_t port) {
        uint8_t ret;
        asm volatile ( "inb %1, %0" : "=a"(ret) : "Nd"(port) );
        return ret;
    }

    void init() {
        outb(0x3F8 + 1, 0x00);
        outb(0x3F8 + 3, 0x80);
        outb(0x3F8 + 0, 0x03);
        outb(0x3F8 + 1, 0x00);
        outb(0x3F8 + 3, 0x03);
        outb(0x3F8 + 2, 0xC7);
        outb(0x3F8 + 4, 0x0B);
    }

    void putchar(char c) {
        int timeout = 100000; // Таймаут, чтобы не висеть вечно
        while ((inb(0x3F8 + 5) & 0x20) == 0 && timeout > 0) {
            timeout--;
        }
        if (timeout > 0) {
            outb(0x3F8, c);
        }
    }

    void puts(const char* s) {
        while (*s) putchar(*s++);
    }

    void put_hex(uint32_t val) {
        char buf[9];
        const char* hex = "0123456789ABCDEF";
        for (int i = 7; i >= 0; i--) {
            buf[i] = hex[val & 0xF];
            val >>= 4;
        }
        buf[8] = '\0';
        puts(buf);
    }

    void put_hex64(uint64_t val) {
        put_hex(val >> 32);
        put_hex(val & 0xFFFFFFFF);
    }
}