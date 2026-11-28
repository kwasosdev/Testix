// include/panic.hpp

#pragma once
#include <stdint.h>

extern "C" void panic(const char* msg);
void panic_hex(const char* msg, uint64_t code, uint64_t rip);