// arch/x86_64/paging/paging.cpp

#include "paging_internal.hpp"
#include <serial.hpp>

namespace paging {
    uint64_t hhdm_offset = 0;

    void set_hhdm_offset(uint64_t offset) {
        hhdm_offset = offset;
        serial::puts("Testix: HHDM offset set.\n");
    }

    uint64_t get_hhdm_offset() {
        return hhdm_offset;
    }
}
