// drivers/video/console/puts.cpp

#include "console_internal.hpp"

namespace console {
    void puts(const char* s) {
        while (*s) putchar(*s++);
    }
}
