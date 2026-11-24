#pragma once
#include <stdint.h>

namespace syscall {
    void init();
}

// Ассемблерная функция для прыжка в Ring 3
extern "C" void enter_usermode(uint64_t entry_point, uint64_t user_stack);