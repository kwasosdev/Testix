// include/loader.hpp

#pragma once
#include <stdint.h>
#include <stddef.h>

extern "C" const uint8_t prog_a_bin[];
extern "C" const uint8_t prog_a_end[];
extern "C" const uint8_t prog_b_bin[];
extern "C" const uint8_t prog_b_end[];
extern "C" const uint8_t shell_bin[]; extern "C" const uint8_t shell_end[];

namespace loader {
    bool spawn_flat(const uint8_t* binary, size_t size);
}
