// arch/x86_64/interrupts/tss.cpp

#include <tss.hpp>
#include <serial.hpp>
#include <types.hpp>

TSS kernel_tss;
static uint8_t kernel_stack[8192] __attribute__((aligned(16))); // Увеличили стек до 8 КБ

void tss::init() {
    uint8_t* tss_ptr = (uint8_t*)&kernel_tss;
    for (uint64_t i = 0; i < sizeof(TSS); i++) {
        tss_ptr[i] = 0;
    }

    kernel_tss.rsp0 = (uint64_t)kernel_stack + sizeof(kernel_stack);
    kernel_tss.iopb_offset = sizeof(TSS);

    serial::puts("Testix: TSS filled OK.\n");
}