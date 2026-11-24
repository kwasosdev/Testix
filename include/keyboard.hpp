#pragma once
#include <stdint.h>

namespace keyboard {
    void init();
    void handle_interrupt();
    char getchar();
}