// arch/x86_64/paging/map_page.cpp

#include "paging_internal.hpp"
#include <pmm.hpp>
#include <panic.hpp>

namespace paging {
    void map_page(uint64_t* pml4_phys, uint64_t virt_addr, uint64_t phys_addr, uint64_t flags) {
        uint64_t* pml4 = (uint64_t*)((uint64_t)pml4_phys + hhdm_offset);
        
        uint64_t pml4_idx = (virt_addr >> 39) & 0x1FF;
        uint64_t pdpt_idx = (virt_addr >> 30) & 0x1FF;
        uint64_t pd_idx   = (virt_addr >> 21) & 0x1FF;
        uint64_t pt_idx   = (virt_addr >> 12) & 0x1FF;

        // PML4
        if (!(pml4[pml4_idx] & PAGE_PRESENT)) {
            uint64_t pdpt_phys = (uint64_t)pmm::alloc_page();
            if (!pdpt_phys) panic("VMM: Out of memory for PDPT!");
            uint64_t* pdpt = (uint64_t*)(pdpt_phys + hhdm_offset);
            for (int i = 0; i < 512; i++) pdpt[i] = 0;
            pml4[pml4_idx] = pdpt_phys | flags | PAGE_PRESENT | PAGE_WRITE;
        } else if (flags & PAGE_USER) {
            pml4[pml4_idx] |= PAGE_USER;
        }
        uint64_t* pdpt = (uint64_t*)((pml4[pml4_idx] & ~0xFFFULL) + hhdm_offset);

        // PDPT
        if (!(pdpt[pdpt_idx] & PAGE_PRESENT)) {
            uint64_t pd_phys = (uint64_t)pmm::alloc_page();
            if (!pd_phys) panic("VMM: Out of memory for PD!");
            uint64_t* pd = (uint64_t*)(pd_phys + hhdm_offset);
            for (int i = 0; i < 512; i++) pd[i] = 0;
            pdpt[pdpt_idx] = pd_phys | flags | PAGE_PRESENT | PAGE_WRITE;
        } else if (flags & PAGE_USER) {
            pdpt[pdpt_idx] |= PAGE_USER;
        }
        uint64_t* pd = (uint64_t*)((pdpt[pdpt_idx] & ~0xFFFULL) + hhdm_offset);

        // PD
        if (!(pd[pd_idx] & PAGE_PRESENT)) {
            uint64_t pt_phys = (uint64_t)pmm::alloc_page();
            if (!pt_phys) panic("VMM: Out of memory for PT!");
            uint64_t* pt = (uint64_t*)(pt_phys + hhdm_offset);
            for (int i = 0; i < 512; i++) pt[i] = 0;
            pd[pd_idx] = pt_phys | flags | PAGE_PRESENT | PAGE_WRITE;
        } else if (flags & PAGE_USER) {
            pd[pd_idx] |= PAGE_USER;
        }
        uint64_t* pt = (uint64_t*)((pd[pd_idx] & ~0xFFFULL) + hhdm_offset);

        // PT
        pt[pt_idx] = (phys_addr & ~0xFFFULL) | flags | PAGE_PRESENT;
        
        asm volatile("invlpg (%0)" :: "r"(virt_addr) : "memory");
    }
}
