#pragma once
#include <stdint.h>

struct Task {
    uint64_t rsp;
    uint64_t cr3;
    uint64_t kernel_stack_top;
    uint64_t kernel_stack_phys;
    uint32_t pid;
    bool used;
    bool is_user;
};

extern "C" uint64_t switch_rsp;
extern "C" uint64_t switch_cr3;

namespace task {
    void init();
    Task* register_current();
    Task* create_kernel_task(void (*entry)());
    Task* create_user_task(uint64_t entry, uint64_t user_stack, uint64_t cr3);
    void schedule_from_irq(uint64_t current_rsp);
    Task* get_current();
}