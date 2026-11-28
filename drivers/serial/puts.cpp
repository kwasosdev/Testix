// drivers/serial/puts.cpp

#include "serial_internal.hpp"

namespace serial {
    void puts(const char* s) {
        while (*s) putchar(*s++);
    }
}
