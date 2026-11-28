// kernel/mm/pmm.cpp

#include <pmm.hpp>
#include <limine.hpp>
#include <serial.hpp>

#define PAGE_SIZE 4096

static uint8_t bitmap[1048576 / 8]; 
static uint64_t total_pages = 0;

static inline void bitmap_set(uint64_t bit) {
    bitmap[bit / 8] |= (1 << (bit % 8));
}

static inline void bitmap_clear(uint64_t bit) {
    bitmap[bit / 8] &= ~(1 << (bit % 8));
}

static inline bool bitmap_test(uint64_t bit) {
    return bitmap[bit / 8] & (1 << (bit % 8));
}

void pmm::init(struct limine_memmap_response* memmap) {
    for (uint64_t i = 0; i < sizeof(bitmap); i++) {
        bitmap[i] = 0xFF;
    }

    for (uint64_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry* entry = memmap->entries[i];
        
        if (entry->type == LIMINE_MEMMAP_USABLE) {
            uint64_t start_page = entry->base / PAGE_SIZE;
            uint64_t end_page = (entry->base + entry->length) / PAGE_SIZE;
            
            for (uint64_t j = start_page; j < end_page; j++) {
                if (j < sizeof(bitmap) * 8) {
                    bitmap_clear(j);
                }
            }
        }
    }
    
    for (uint64_t i = 0; i < (0x200000 / PAGE_SIZE); i++) {
        bitmap_set(i);
    }

    total_pages = sizeof(bitmap) * 8;
    serial::puts("Testix: PMM Initialized.\n");
}

void* pmm::alloc_page() {
    for (uint64_t i = 0; i < total_pages; i++) {
        if (!bitmap_test(i)) {
            bitmap_set(i);
            return (void*)(i * PAGE_SIZE);
        }
    }
    return nullptr;
}

void pmm::free_page(void* ptr) {
    uint64_t addr = (uint64_t)ptr;
    uint64_t page = addr / PAGE_SIZE;
    if (page < total_pages) {
    bitmap_clear(page);
}
}
