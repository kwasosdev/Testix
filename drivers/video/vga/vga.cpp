#include <vga.hpp>
#include <font8x8.hpp>

namespace vga {

static constexpr size_t VGA_WIDTH  = 80;
static constexpr size_t VGA_HEIGHT = 50; // Режим 80x50
static volatile uint16_t* const VGA_BUFFER =
    reinterpret_cast<volatile uint16_t*>(0xB8000);

static uint8_t color = 0x0F; // Белый на черном
static size_t row = 0;
static size_t col = 0;

// Вспомогательные функции для работы с портами (I/O)
static inline void outb(uint16_t port, uint8_t val) {
    asm volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) );
}
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ( "inb %1, %0" : "=a"(ret) : "Nd"(port) );
    return ret;
}

// Функции для работы с регистрами VGA
static void write_seq(uint8_t index, uint8_t data) { outb(0x3C4, index); outb(0x3C5, data); }
static uint8_t read_seq(uint8_t index) { outb(0x3C4, index); return inb(0x3C5); }
static void write_gc(uint8_t index, uint8_t data) { outb(0x3CE, index); outb(0x3CF, data); }
static uint8_t read_gc(uint8_t index) { outb(0x3CE, index); return inb(0x3CF); }

// Обновление аппаратного курсора
static void update_cursor() {
    uint16_t pos = row * VGA_WIDTH + col;
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

// Загрузка кастомного шрифта в память видеокарты
void load_custom_font() {
    uint8_t seq1 = read_seq(0x01);
    uint8_t seq2 = read_seq(0x02);
    uint8_t seq4 = read_seq(0x04);
    uint8_t gc4 = read_gc(0x04);
    uint8_t gc5 = read_gc(0x05);
    uint8_t gc6 = read_gc(0x06);

    write_seq(0x01, seq1 | 0x20); // Отключаем экран
    write_seq(0x02, 0x04);        // Map Mask (Plane 2)
    write_seq(0x04, seq4 | 0x04); // Memory Mode

    write_gc(0x04, 0x02);         // Read Map Select (Plane 2)
    write_gc(0x05, 0x00);         // Mode
    write_gc(0x06, 0x04);         // Misc

    volatile uint8_t* vga_mem = (volatile uint8_t*)0xA0000;
    for (int c = 0; c < 256; c++) {
        const uint8_t* glyph = font::get_glyph(c);
        for (int r = 0; r < 16; r++) { // В VGA всегда резервируется 16 байт под символ
            if (r < 8 && glyph) {
                vga_mem[c * 32 + r] = glyph[r];
            } else {
                vga_mem[c * 32 + r] = 0x00;
            }
        }
    }

    // Восстанавливаем регистры
    write_gc(0x04, gc4);
    write_gc(0x05, gc5);
    write_gc(0x06, gc6);
    write_seq(0x04, seq4);
    write_seq(0x02, seq2);
    write_seq(0x01, seq1); // Включаем экран
}

void init() { 
    load_custom_font(); 
    clear();
    
    // Прячем мигающий курсор BIOS, чтобы он не мешал
    outb(0x3D4, 0x0A);
    outb(0x3D5, 0x20); 
}

void clear() {
    // Принудительно забиваем ВСЕ 4000 символов (80 * 50) пустотой
    for (size_t i = 0; i < VGA_WIDTH * VGA_HEIGHT; ++i) {
        VGA_BUFFER[i] = (color << 8) | ' ';
    }
    row = 0;
    col = 0;
    update_cursor();
}

void putchar(char c) {
    if (c == '\n') { 
        col = 0; 
        ++row; 
    }
    else if (c == '\r') { 
        col = 0; 
    }
    else {
        VGA_BUFFER[row * VGA_WIDTH + col] = (color << 8) | static_cast<uint8_t>(c);
        if (++col >= VGA_WIDTH) { col = 0; ++row; }
    }
    
    // Если дошли до низа экрана, сбрасываем наверх (простой скролл)
    if (row >= VGA_HEIGHT) row = 0; 
    update_cursor();
}

void puts(const char* s) {
    while (*s) putchar(*s++);
}

} // namespace vga