// include/pit.hpp

#pragma once
#include <stdint.h>

namespace pit {
    void init(uint32_t frequency);
    uint64_t get_ticks();
    void increment_ticks();
}