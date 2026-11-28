// include/loader.hpp

#pragma once
#include <stdint.h>
#include <stddef.h>

namespace loader {
    bool spawn_flat(const uint8_t* binary, size_t size, const char* args = "");
    bool spawn_file(const char* path, const char* args = "");
}
