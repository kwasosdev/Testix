// include/pic.hpp

#pragma once
#include <stdint.h>

namespace pic {
    void remap();
    void send_eoi(uint8_t irq);
}