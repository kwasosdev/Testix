#include <panic.hpp>
#include <serial.hpp>
#include <framebuffer.hpp>

static void print_hex_to_fb(uint64_t val, uint32_t x, uint32_t y) {
    char buf[17];
    const char* hex = "0123456789ABCDEF";
    for (int i = 15; i >= 0; i--) {
        buf[i] = hex[val & 0xF];
        val >>= 4;
    }
    buf[16] = '\0';
    fb::puts("0x", x, y, 0xFFFFFF, 2);
    fb::puts(buf, x + 32, y, 0xFFFFFF, 2);
}

static bool panicking = false;

extern "C" void panic(const char* msg) {
    asm volatile("cli");
    if (panicking) {
        for (;;) asm volatile("hlt");
    }
    panicking = true;

    // 1. Serial
    serial::puts("\n\n*** KERNEL PANIC ***\n");
    serial::puts(msg);
    serial::puts("\nSystem Halted.\n");

    // 2. Framebuffer
    if (fb::is_initialized()) {
        uint32_t w = fb::get_width();
        uint32_t h = fb::get_height();
        
        fb::fill_rect(0, 0, w, h, 0xAA0000); // Заливаем реальный размер экрана
        fb::puts("*** KERNEL PANIC ***", 50, 50, 0xFFFFFF, 4);
        fb::puts(msg, 50, 120, 0xFFFFFF, 2);
        fb::puts("System Halted.", 50, 160, 0xFFFFFF, 2);
    }

    for (;;) asm volatile("hlt");
}

void panic_hex(const char* msg, uint64_t code, uint64_t rip) {
    asm volatile("cli");
    if (panicking) {
        for (;;) asm volatile("hlt");
    }
    panicking = true;

    // 1. Serial
    serial::puts("\n\n*** KERNEL PANIC ***\n");
    serial::puts(msg);
    serial::puts("\nError Code: "); serial::put_hex64(code);
    serial::puts("\nRIP: "); serial::put_hex64(rip);
    serial::puts("\nSystem Halted.\n");

    // 2. Framebuffer
    if (fb::is_initialized()) {
        uint32_t w = fb::get_width();
        uint32_t h = fb::get_height();
        
        fb::fill_rect(0, 0, w, h, 0xAA0000);
        fb::puts("*** KERNEL PANIC ***", 50, 50, 0xFFFFFF, 4);
        fb::puts(msg, 50, 120, 0xFFFFFF, 2);
        
        fb::puts("Error Code:", 50, 160, 0xFFFFFF, 2);
        print_hex_to_fb(code, 250, 160);
        
        fb::puts("RIP:", 50, 200, 0xFFFFFF, 2);
        print_hex_to_fb(rip, 250, 200);
        
        fb::puts("System Halted.", 50, 260, 0xFFFFFF, 2);
    }

    for (;;) asm volatile("hlt");
}