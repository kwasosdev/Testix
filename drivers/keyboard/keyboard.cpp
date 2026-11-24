#include <keyboard.hpp>
#include <pic.hpp>

namespace keyboard {
    static inline uint8_t inb(uint16_t port) {
        uint8_t ret;
        asm volatile ( "inb %1, %0" : "=a"(ret) : "Nd"(port) );
        return ret;
    }

    static const char keymap[128] = {
        0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
        '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
        0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
        0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
        '*', 0, ' '
    };

    static const char keymap_shift[128] = {
        0,  27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
        '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
        0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',
        0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
        '*', 0, ' '
    };

    static bool shift_pressed = false;
    static char buffer[256];
    static int buf_head = 0;
    static int buf_tail = 0;

    void init() {}

    void handle_interrupt() {
        uint8_t scancode = inb(0x60);

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
                buf_head = (buf_head + 1) % 256;
            }
        }
    }

    char getchar() {
        if (buf_head == buf_tail) return 0;
        char c = buffer[buf_tail];
        buf_tail = (buf_tail + 1) % 256;
        return c;
    }
}