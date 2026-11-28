// arch/x86_64/paging/paging_internal.hpp

#pragma once
#include <paging.hpp>

namespace paging {
    extern uint64_t hhdm_offset;

    static inline uint64_t read_cr3() {
        uint64_t cr3;
        asm volatile("mov %%cr3, %0" : "=r"(cr3));
        return cr3;
    }

    static inline void write_cr3(uint64_t cr3) {
        asm volatile("mov %0, %%cr3" :: "r"(cr3) : "memory");
    }
}
