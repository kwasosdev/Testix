// drivers/video/vga/load_custom_font.cpp

#include "vga_internal.hpp"

namespace vga {
  void load_custom_font() {
      uint8_t seq1 = vga::read_seq(0x01);
      uint8_t seq2 = vga::read_seq(0x02);
      uint8_t seq4 = vga::read_seq(0x04);
      uint8_t gc4 = vga::read_gc(0x04);
      uint8_t gc5 = vga::read_gc(0x05);
      uint8_t gc6 = vga::read_gc(0x06);

      vga::write_seq(0x01, seq1 | 0x20); // Отключаем экран
      vga::write_seq(0x02, 0x04);        // Map Mask (Plane 2)
      vga::write_seq(0x04, seq4 | 0x04); // Memory Mode

      vga::write_gc(0x04, 0x02);         // Read Map Select (Plane 2)
      vga::write_gc(0x05, 0x00);         // Mode
      vga::write_gc(0x06, 0x04);         // Misc

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
      vga::write_gc(0x04, gc4);
      vga::write_gc(0x05, gc5);
      vga::write_gc(0x06, gc6);
      vga::write_seq(0x04, seq4);
      vga::write_seq(0x02, seq2);
      vga::write_seq(0x01, seq1); // Включаем экран
  }
}