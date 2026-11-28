// drivers/serial/put_hex.cpp

#include "serial_internal.hpp"

namespace serial {
    void put_hex(uint32_t val) {
        char buf[9];
        const char* hex = "0123456789ABCDEF";
        for (int i = 7; i >= 0; i--) {
            buf[i] = hex[val & 0xF];
            val >>= 4;
        }
        buf[8] = '\0';
        puts(buf);
    }

    void put_hex64(uint64_t val) {
        put_hex(val >> 32);
        put_hex(val & 0xFFFFFFFF);
    }
}
