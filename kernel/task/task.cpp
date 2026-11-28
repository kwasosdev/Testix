// task.cpp

#include <task.hpp>
#include <pmm.hpp>
#include <paging.hpp>
#include <serial.hpp>
#include <panic.hpp>
#include <tss.hpp>

// Глобальные переменные, которые ищет линкер из ассемблера
uint64_t switch_rsp = 0;
uint64_t switch_cr3 = 0;

static Task tasks[16];
static Task* current_task = nullptr;
static int next_pid = 1;
static uint64_t tick_count = 0;
static const uint64_t QUANTUM = 3;

void task::init() {
    for (int i = 0; i < 16; i++) tasks[i].used = false;
    current_task = nullptr;
    next_pid = 1;
    tick_count = 0;
}

static Task* alloc_task() {
    for (int i = 0; i < 16; i++) {
        if (!tasks[i].used) {
            tasks[i].used = true;
            tasks[i].pid = next_pid++;
            tasks[i].rsp = 0;
            tasks[i].cr3 = 0;
            tasks[i].is_user = false;
            return &tasks[i];
        }
    }
    panic("task: no free slots");
    return nullptr;
}

static Task* build_task(uint64_t entry, uint64_t user_rsp, uint64_t cs, uint64_t ss) {
    uint64_t stack_phys = (uint64_t)pmm::alloc_page();
    uint64_t hhdm = paging::get_hhdm_offset();
    uint64_t stack_top = stack_phys + hhdm + 4096;
    uint64_t* sp = (uint64_t*)stack_top;

    *--sp = ss;
    *--sp = user_rsp;
    *--sp = 0x202;
    *--sp = cs;
    *--sp = entry;

    *--sp = 0;
    *--sp = 0;

    for (int i = 0; i < 15; i++) *--sp = 0;

    Task* t = alloc_task();
    t->rsp = (uint64_t)sp;
    t->kernel_stack_top = stack_top;
    t->kernel_stack_phys = stack_phys;
    return t;
}

Task* task::register_current() {
    Task* t = alloc_task();
    t->is_user = false;
    t->cr3 = 0;
    t->rsp = 0;
    current_task = t;
    return t;
}

Task* task::create_kernel_task(void (*entry)()) {
    uint64_t task_stack_phys = (uint64_t)pmm::alloc_page();
    uint64_t hhdm = paging::get_hhdm_offset();
    uint64_t task_stack_top = task_stack_phys + hhdm + 4096;

    Task* t = build_task((uint64_t)entry, task_stack_top, 0x08, 0x10);
    t->is_user = false;
    return t;
}

Task* task::create_user_task(uint64_t entry, uint64_t user_stack, uint64_t cr3) {
    Task* t = build_task(entry, user_stack, 0x1B, 0x23);
    t->is_user = true;
    t->cr3 = cr3;
    return t;
}

Task* task::get_current() {
    return current_task;
}

void task::yield(uint64_t current_rsp) {
    tick_count = QUANTUM - 1;
    schedule_from_irq(current_rsp);
}

void task::schedule_from_irq(uint64_t current_rsp) {
    if (!current_task) return;
    current_task->rsp = current_rsp;

    tick_count++;
    if (tick_count % QUANTUM != 0) return;

    int start = 0;
    for (int i = 0; i < 16; i++) {
        if (&tasks[i] == current_task) { start = i + 1; break; }
    }
    Task* next = nullptr;
    for (int j = 0; j < 16; j++) {
        int i = (start + j) % 16;
        if (tasks[i].used && &tasks[i] != current_task) {
            next = &tasks[i];
            break;
        }
    }
    if (!next) return;

    serial::puts("[sched] switching to pid ");
    serial::put_hex64(next->pid);
    serial::puts("\n");

    if (next->is_user) {
        kernel_tss.rsp0 = next->kernel_stack_top;
    }

    current_task = next;
    switch_rsp = next->rsp;
    switch_cr3 = next->cr3;
}

void task::sleep_current() {
    if (current_task) {
        current_task->used = false;
    }
}

void task::wake_shell() {
    // Будим первую задачу (pid 1 — shell)
    if (tasks[0].pid == 1) {
        tasks[0].used = true;
    }
}

void task::exit_current(uint64_t current_rsp) {
    if (!current_task) return;
    current_task->used = false; // Освобождаем слот текущей user-задачи

    // Проверяем, остались ли ещё живые задачи
    bool any_alive = false;
    for (int i = 0; i < 16; i++) {
        if (tasks[i].used) {
            any_alive = true;
            break;
        }
    }

    // Если все user-задачи завершились — возвращаем управление в shell
    if (!any_alive) {
        wake_shell();
    }

    tick_count = QUANTUM - 1;
    schedule_from_irq(current_rsp);
}

int task::active_user_tasks() {
    int count = 0;
    for (int i = 0; i < 16; i++) {
        if (tasks[i].used && tasks[i].is_user) {
            count++;
        }
    }
    return count;
}