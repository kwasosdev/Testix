/* linker.ld */

OUTPUT_FORMAT(elf64-x86-64)
OUTPUT_ARCH(i386:x86-64)
ENTRY(_start)

KERNEL_VMA   = 0xffffffff80000000;
KERNEL_LMA   = 0x200000; 
KERNEL_DELTA = KERNEL_VMA - KERNEL_LMA;

PHDRS
{
    limine_requests PT_LOAD;
    text PT_LOAD;
    rodata PT_LOAD;
    data PT_LOAD;
}

SECTIONS
{
    . = KERNEL_VMA;

    .limine_requests : {
        KEEP(*(.limine_requests_start))
        KEEP(*(.limine_requests))
        KEEP(*(.limine_requests_end))
    } :limine_requests

    . = ALIGN(4K);

    .text : AT(ADDR(.text) - KERNEL_DELTA) {
        KEEP(*(.text._start))
        *(.text*)
    } :text

    . = ALIGN(4K);

    .rodata : AT(ADDR(.rodata) - KERNEL_DELTA) {
        *(.rodata*)
    } :rodata

    . = ALIGN(4K);

    .data : AT(ADDR(.data) - KERNEL_DELTA) {
        *(.data*)
    } :data

    . = ALIGN(4K);

    .bss : AT(ADDR(.bss) - KERNEL_DELTA) {
        __bss_start = .;
        *(COMMON)
        *(.bss*)
        __bss_end = .;
    } :data

    /DISCARD/ :
    {
        *(.comment)
        *(.eh_frame)
        *(.note*)
    }
}