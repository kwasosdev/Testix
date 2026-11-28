// arch/x86_64/paging/switch_address_space.cpp

#include "paging_internal.hpp"
#include <serial.hpp>
#include <panic.hpp>

namespace paging {
    void switch_address_space(uint64_t* pml4_phys) {
        if (!pml4_phys) panic("VMM: Attempted to switch to null PML4!");
        write_cr3((uint64_t)pml4_phys);
        serial::puts("Testix: Switched address space (CR3 updated).\n");
    }
}
