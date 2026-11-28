// include/pmm.hpp

#pragma once

// Предварительное объявление, чтобы не тянуть весь limine.hpp
struct limine_memmap_response;

namespace pmm {
    void init(struct limine_memmap_response* memmap);
    void* alloc_page();
    void free_page(void* ptr);
}