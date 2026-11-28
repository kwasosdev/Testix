/* user/user.ld */

ENTRY(_start)

SECTIONS
{
    . = 0x400000;

    .text : {
        *(.text._start)
        *(.text*)
    }

    .rodata : ALIGN(16) {
        *(.rodata*)
    }

    .data : ALIGN(16) {
        *(.data*)
    }

    .bss : ALIGN(16) {
        *(COMMON)
        *(.bss*)
        BYTE(0) /* Гарантирует, что .bss физически попадёт в flat-бинарник и обнулится */
    }
}
