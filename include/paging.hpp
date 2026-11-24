#pragma once
#include <stdint.h>

#define PAGE_PRESENT 0x01
#define PAGE_WRITE   0x02
#define PAGE_USER    0x04

namespace paging {
    void set_hhdm_offset(uint64_t offset); // <--- НОВОЕ
    uint64_t* create_address_space();
    uint64_t get_hhdm_offset();
    void map_page(uint64_t* pml4_phys, uint64_t virt, uint64_t phys, uint64_t flags);
    void switch_address_space(uint64_t* pml4_phys);
}