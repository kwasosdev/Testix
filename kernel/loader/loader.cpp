// kernel/loader/loader.cpp

#include <loader.hpp>
#include <paging.hpp>
#include <pmm.hpp>
#include <task.hpp>
#include <serial.hpp>
#include <vfs.hpp>

static constexpr uint64_t USER_CODE_BASE  = 0x400000;
static constexpr uint64_t USER_STACK_BASE = 0x7FFF0000;
static constexpr uint64_t PAGE_SIZE       = 4096;

bool loader::spawn_flat(const uint8_t* binary, size_t size, const char* args) {
    if (!binary || size == 0) return false;

    uint64_t hhdm = paging::get_hhdm_offset();
    uint64_t* user_pml4 = paging::create_address_space();

    // 1. Мапим и копируем страницы кода и данных (0x400000)
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

    // 2. Выделяем 2 страницы под стек (8 КБ) и кладём строку аргументов на вершину верхней страницы
    uint64_t top_stack_phys = 0;
    for (size_t s = 0; s < 2; s++) {
        void* stack_phys = pmm::alloc_page();
        if (!stack_phys) return false;
        paging::map_page(user_pml4, USER_STACK_BASE + s * PAGE_SIZE, (uint64_t)stack_phys, PAGE_USER | PAGE_WRITE);
        if (s == 1) top_stack_phys = (uint64_t)stack_phys;
    }

    // Копируем строку args в последние 256 байт стека (выровненную по 16 байт)
    const char* safe_args = args ? args : "";
    size_t arg_len = 0;
    while (safe_args[arg_len] && arg_len < 250) arg_len++;

    uint64_t args_offset = PAGE_SIZE - 256;
    uint8_t* stack_virt_kernel = (uint8_t*)(top_stack_phys + hhdm + args_offset);
    for (size_t i = 0; i < arg_len; i++) stack_virt_kernel[i] = (uint8_t)safe_args[i];
    stack_virt_kernel[arg_len] = 0;

    uint64_t user_args_ptr = USER_STACK_BASE + PAGE_SIZE + args_offset;
    uint64_t user_rsp_top  = user_args_ptr - 16; // Стек растёт вниз ниже буфера аргументов

    uint64_t cr3_phys = (uint64_t)user_pml4;
    if (cr3_phys >= hhdm) cr3_phys -= hhdm;

    // 3. Создаём задачу с передачей user_args_ptr в регистр RDI
    task::create_user_task(USER_CODE_BASE, user_rsp_top, cr3_phys, user_args_ptr);
    return true;
}

bool loader::spawn_file(const char* path, const char* args) {
    static uint8_t exec_buf[vfs::MAX_FILE_SIZE];
    int bytes = vfs::read(path, exec_buf, sizeof(exec_buf));
    if (bytes <= 0) return false;
    return spawn_flat(exec_buf, (size_t)bytes, args);
}
