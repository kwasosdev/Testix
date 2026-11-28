; arch/x86_64/entry.asm

[bits 64]
[extern kmain]
[extern __bss_start]
[extern __bss_end]
[global _start]

section .text._start
_start:
    cli
    cld

    ; УБРАЛИ mov rsp, 0x90000 - теперь используется безопасный стек от Limine!

    mov r12, rdi        ; Сохраняем boot_info

    ; Zero .bss
    mov rdi, __bss_start
    mov rcx, __bss_end
    sub rcx, rdi
    xor al, al
    rep stosb

    mov rdi, r12        ; Восстанавливаем boot_info

    call kmain

.hang:
    cli
    hlt
    jmp .hang