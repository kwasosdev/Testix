// arch/x86_64/interrupts/pic.cpp

#include <pic.hpp>
#include <stdint.h>

namespace pic {

static inline void outb(uint16_t port, uint8_t val) {
    asm volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) );
}
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ( "inb %1, %0" : "=a"(ret) : "Nd"(port) );
    return ret;
}

void remap() {
    // Начинаем инициализацию
    outb(0x20, 0x11);
    outb(0xA0, 0x11);

    // Смещаем Master PIC на 0x20 (32-й вектор), Slave на 0x28 (40-й)
    outb(0x21, 0x20);
    outb(0xA1, 0x28);

    // Сообщаем, что Slave подключен к IRQ2 Master'а
    outb(0x21, 0x04);
    outb(0xA1, 0x02);

    // Режим 8086
    outb(0x21, 0x01);
    outb(0xA1, 0x01);

    // Разрешаем только IRQ1 (Клавиатура) для Master
    // и запрещаем все для Slave (на будущее)
    outb(0x21, 0xFC);
    outb(0xA1, 0xFF); // 11111111 - всё запрещено
}

void send_eoi(uint8_t irq) {
    if (irq >= 8)
        outb(0xA0, 0x20);
    outb(0x20, 0x20);
}

} // namespace pic