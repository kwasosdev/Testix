#include <pit.hpp>

namespace pit {
    static inline void outb(uint16_t port, uint8_t val) {
        asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
    }
    static uint64_t ticks = 0;
    void init(uint32_t freq) {
        uint32_t divisor = 1193182 / freq;
        outb(0x43, 0x36);
        outb(0x40, divisor & 0xFF);
        outb(0x40, (divisor >> 8) & 0xFF);
    }
    uint64_t get_ticks() { return ticks; }
    void increment_ticks() { ticks++; }
}