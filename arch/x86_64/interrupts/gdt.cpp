// arch/x86_64/interrupts/gdt.cpp

#include <gdt.hpp>
#include <tss.hpp>
#include <serial.hpp>
#include <types.hpp>

struct TSSEntry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_middle;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  base_high;
    uint32_t base_upper32;
    uint32_t reserved;
} __attribute__((packed));

struct GDTTable {
    GDTEntry entries[5];
    TSSEntry tss;
} __attribute__((packed));

static GDTTable gdt;
static GDTPointer gdtp;

void gdt_init() {
    gdt.entries[0] = {0, 0, 0, 0, 0, 0};
    gdt.entries[1] = {0, 0, 0, 0x9A, 0xAF, 0}; // Kernel Code
    gdt.entries[2] = {0, 0, 0, 0x92, 0xCF, 0}; // Kernel Data
    gdt.entries[3] = {0, 0, 0, 0xFA, 0xAF, 0}; // User Code
    gdt.entries[4] = {0, 0, 0, 0xF2, 0xCF, 0}; // User Data

    uint64_t tss_base = (uint64_t)&kernel_tss;
    uint32_t tss_limit = sizeof(TSS) - 1;

    gdt.tss.limit_low = tss_limit & 0xFFFF;
    gdt.tss.base_low = tss_base & 0xFFFF;
    gdt.tss.base_middle = (tss_base >> 16) & 0xFF;
    gdt.tss.access = 0x89; // Present, 64-bit TSS
    gdt.tss.granularity = (tss_limit >> 16) & 0x0F;
    gdt.tss.base_high = (tss_base >> 24) & 0xFF;
    gdt.tss.base_upper32 = (tss_base >> 32) & 0xFFFFFFFF;
    gdt.tss.reserved = 0;

    gdtp.limit = sizeof(GDTTable) - 1;
    gdtp.base = (uint64_t)&gdt;

    load_gdt(&gdtp, 0x28);
}