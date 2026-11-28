// arch/x86_64/paging/create_address_space.cpp

#include "paging_internal.hpp"
#include <pmm.hpp>
#include <serial.hpp>
#include <panic.hpp>

namespace paging {
    uint64_t* create_address_space() {
        uint64_t phys = (uint64_t)pmm::alloc_page();
        if (!phys) panic("VMM: Out of memory for new PML4!");
        
        uint64_t* virt = (uint64_t*)(phys + hhdm_offset);
        for (int i = 0; i < 512; i++) virt[i] = 0;

        uint64_t old_cr3 = read_cr3();
        uint64_t* old_virt = (uint64_t*)(old_cr3 + hhdm_offset);
        
        for (int i = 256; i < 512; i++) {
            virt[i] = old_virt[i];
        }

        serial::puts("Testix: Created new address space.\n");
        return (uint64_t*)phys;
    }
}
