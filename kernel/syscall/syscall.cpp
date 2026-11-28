// kernel/syscall/syscall.cpp
#include <syscall.hpp>
#include <console.hpp>
#include <keyboard.hpp>
#include <loader.hpp>
#include <panic.hpp>
#include <task.hpp>
#include <types.hpp>
#include <idt.hpp>

struct SyscallRegs {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
    uint64_t int_no, err_code;
    uint64_t rip, cs, rflags, rsp, ss;
};

extern "C" void syscall_entry();

extern "C" void syscall_handler_c(SyscallRegs* regs) {
    uint64_t num = regs->rax;
    
    if (num == 1) {
        console::putchar((char)regs->rdi);
        regs->rax = 1;
    } 
    else if (num == 2) {
        regs->rax = 0;
        task::yield((uint64_t)regs);
    }
    else if (num == 3) {
        task::exit_current((uint64_t)regs);
    }
    else if (num == 4) {
        regs->rax = (uint64_t)(uint8_t)keyboard::getchar();
    }
    else if (num == 5) {
        console::clear();
        regs->rax = 0;
    }
    else if (num == 6) {
        bool ok = false;
        if (regs->rdi == 1) {
            ok = loader::spawn_flat(prog_a_bin, (size_t)(prog_a_end - prog_a_bin));
        } else if (regs->rdi == 2) {
            ok = loader::spawn_flat(prog_b_bin, (size_t)(prog_b_end - prog_b_bin));
        }
        regs->rax = ok ? 1 : 0;
    }
    else if (num == 7) {
        panic("panictest: triggered from Ring 3!");
    }
    else if (num == 8) {
        regs->rax = (uint64_t)task::active_user_tasks();
    }

    else {
        regs->rax = (uint64_t)-1;
    }
}

void syscall::init() {
    register_syscall_gate(0x80, (void(*)())syscall_entry);
}
