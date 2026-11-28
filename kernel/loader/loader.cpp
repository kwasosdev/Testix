// kernel/loader/loader.cpp
#include <loader.hpp>
#include <paging.hpp>
#include <pmm.hpp>
#include <task.hpp>
#include <serial.hpp>

static constexpr uint64_t USER_CODE_BASE  = 0x400000;
static constexpr uint64_t USER_STACK_BASE = 0x7FFF0000;
static constexpr uint64_t PAGE_SIZE       = 4096;

bool loader::spawn_flat(const uint8_t* binary, size_t size) {
    if (!binary || size == 0) return false;

    uint64_t hhdm = paging::get_hhdm_offset();
    uint64_t* user_pml4 = paging::create_address_space();

    // 1. Выделяем, мапим и копируем страницы кода и данных программы (0x400000)
    size_t code_pages = (size + PAGE_SIZE - 1) / PAGE_SIZE;
    for (size_t p = 0; p < code_pages; p++) {
        void* code_phys = pmm::alloc_page();
        if (!code_phys) return false;

        paging::map_page(user_pml4, USER_CODE_BASE + p * PAGE_SIZE, (uint64_t)code_phys, PAGE_USER | PAGE_WRITE);

        uint8_t* dst = (uint8_t*)((uint64_t)code_phys + hhdm);
        size_t offset = p * PAGE_SIZE;
        size_t chunk = (size - offset > PAGE_SIZE) ? PAGE_SIZE : (size - offset);
        for (size_t i = 0; i < chunk; i++) {
            dst[i] = binary[offset + i];
        }
    }

    // 2. Выделяем и мапим 2 страницы под стек пользователя (8 КБ)
    for (size_t s = 0; s < 2; s++) {
        void* stack_phys = pmm::alloc_page();
        if (!stack_phys) return false;
        paging::map_page(user_pml4, USER_STACK_BASE + s * PAGE_SIZE, (uint64_t)stack_phys, PAGE_USER | PAGE_WRITE);
    }

    uint64_t cr3_phys = (uint64_t)user_pml4;
    if (cr3_phys >= hhdm) cr3_phys -= hhdm;

    // 3. Создаём задачу в планировщике с вершиной 8-килобайтного стека
    task::create_user_task(USER_CODE_BASE, USER_STACK_BASE + 2 * PAGE_SIZE - 16, cr3_phys);
    return true;
}
