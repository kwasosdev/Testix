#include <syscall.hpp>
#include <console.hpp>
#include <serial.hpp> // <--- ДОБАВЬ
#include <idt.hpp>

struct SyscallRegs {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
    uint64_t rip, cs, rflags, rsp, ss;
};

extern "C" void syscall_entry();

extern "C" void syscall_handler_c(SyscallRegs* regs) {
    serial::puts("[syscall] num=");
    serial::put_hex64(regs->rax);
    serial::puts("\n");

    uint64_t num = regs->rax;
    
    if (num == 1) {
        console::putchar((char)regs->rdi);
        regs->rax = 1;
    } 
    else if (num == 2) {
        asm volatile("sti; hlt");
        regs->rax = 0;
    }
    else {
        regs->rax = (uint64_t)-1;
    }
}

void syscall::init() {
    register_syscall_gate(0x80, (void(*)())syscall_entry);
}