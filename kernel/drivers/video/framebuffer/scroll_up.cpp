// kernel/drivers/video/framebuffer/scroll_up.cpp
#include "fb_internal.hpp"

namespace fb {
    void scroll_up(uint32_t pixels, uint32_t bg_color) {
        if (!buffer || pixels == 0) return;
        if (pixels >= height) {
            fill_rect(0, 0, width, height, bg_color);
            return;
        }

        uint32_t rows_to_move = height - pixels;

        if (bpp == 32) {
            uint32_t* ptr = (uint32_t*)buffer;
            for (uint32_t y = 0; y < rows_to_move; y++) {
                uint32_t* dst = ptr + y * pitch;
                const uint32_t* src = ptr + (y + pixels) * pitch;
                for (uint32_t x = 0; x < width; x++) {
                    dst[x] = src[x];
                }
            }
        } else {
            uint32_t bytes_per_pixel = (bpp + 7) / 8;
            uint32_t row_bytes = width * bytes_per_pixel;
            uint32_t pitch_bytes = pitch * bytes_per_pixel;
            for (uint32_t y = 0; y < rows_to_move; y++) {
                uint8_t* dst = buffer + y * pitch_bytes;
                const uint8_t* src = buffer + (y + pixels) * pitch_bytes;
                for (uint32_t x = 0; x < row_bytes; x++) {
                    dst[x] = src[x];
                }
            }
        }

        // Очищаем освободившуюся нижнюю область экрана
        fill_rect(0, rows_to_move, width, pixels, bg_color);
    }
}
