; arch/x86_64/interrupts/interrupts.asm

[bits 64]
DEFAULT REL

global load_gdt
global load_idt
global syscall_entry
global enter_usermode

extern isr_handler
extern irq_handler
extern syscall_handler_c

; --- Исключения (ISR) ---
%macro ISR_NOERRCODE 1
global isr%1
isr%1:
    cli
    push 0
    push %1
    jmp isr_common
%endmacro

%macro ISR_ERRCODE 1
global isr%1
isr%1:
    cli
    push %1
    jmp isr_common
%endmacro

ISR_NOERRCODE 0
ISR_NOERRCODE 1
ISR_NOERRCODE 2
ISR_NOERRCODE 3
ISR_NOERRCODE 4
ISR_NOERRCODE 5
ISR_NOERRCODE 6
ISR_NOERRCODE 7
ISR_ERRCODE   8
ISR_NOERRCODE 9
ISR_ERRCODE   10
ISR_ERRCODE   11
ISR_ERRCODE   12
ISR_ERRCODE   13
ISR_ERRCODE   14
ISR_NOERRCODE 15
ISR_NOERRCODE 16
ISR_ERRCODE   17
ISR_NOERRCODE 18
ISR_NOERRCODE 19
ISR_NOERRCODE 20
ISR_ERRCODE   21
ISR_NOERRCODE 22
ISR_NOERRCODE 23
ISR_NOERRCODE 24
ISR_NOERRCODE 25
ISR_NOERRCODE 26
ISR_NOERRCODE 27
ISR_NOERRCODE 28
ISR_ERRCODE   29
ISR_ERRCODE   30
ISR_NOERRCODE 31

; --- Аппаратные прерывания (IRQ) ---
%macro IRQ 2
global irq%1
irq%1:
    cli
    push 0
    push %2
    jmp irq_common
%endmacro

IRQ 0, 32
IRQ 1, 33
IRQ 2, 34
IRQ 3, 35
IRQ 4, 36
IRQ 5, 37
IRQ 6, 38
IRQ 7, 39
IRQ 8, 40
IRQ 9, 41
IRQ 10, 42
IRQ 11, 43
IRQ 12, 44
IRQ 13, 45
IRQ 14, 46
IRQ 15, 47

; --- Общие обработчики ---
isr_common:
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    mov rdi, rsp
    call isr_handler

    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax

    add rsp, 16
    iretq

extern switch_rsp
extern switch_cr3

irq_common:
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    mov rdi, rsp
    call irq_handler

    ; Проверяем, не просит ли нас планировщик переключиться
    mov rax, [switch_cr3]
    test rax, rax
    jz .no_cr3
    mov cr3, rax
    mov qword [switch_cr3], 0
.no_cr3:

    mov rax, [switch_rsp]
    test rax, rax
    jz .no_switch
    mov rsp, rax
    mov qword [switch_rsp], 0
.no_switch:

    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax

    add rsp, 16
    iretq

; --- Загрузка таблиц ---
load_gdt:
    lgdt [rdi]
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    push 0x08
    lea rax, [rel .reload_cs]
    push rax
    retfq
.reload_cs:
    mov ax, si
    ltr ax
    ret

load_idt:
    lidt [rdi]
    ret

; --- Системный вызов ---
syscall_entry:
    cli
    push 0
    push 0x80

    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
    
    mov rdi, rsp
    call syscall_handler_c

    mov rax, [switch_cr3]
    test rax, rax
    jz .no_sys_cr3
    mov cr3, rax
    mov qword [switch_cr3], 0
.no_sys_cr3:

    mov rax, [switch_rsp]
    test rax, rax
    jz .no_sys_switch
    mov rsp, rax
    mov qword [switch_rsp], 0
.no_sys_switch:
    
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax

    add rsp, 16
    iretq

; --- Прыжок в Ring 3 ---
enter_usermode:
    mov ax, 0x23
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push 0x23
    push rsi
    push 0x202
    push 0x1B
    push rdi
    iretq