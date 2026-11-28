// kernel/syscall/syscall.cpp
#include <syscall.hpp>
#include <console.hpp>
#include <keyboard.hpp>
#include <loader.hpp>
#include <panic.hpp>
#include <task.hpp>
#include <types.hpp>
#include <idt.hpp>
#include <vfs.hpp>

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
        // sys_spawn(const char* path, const char* args)
        regs->rax = loader::spawn_file((const char*)regs->rdi, (const char*)regs->rsi) ? 1 : 0;
    }
    else if (num == 7) {
        panic("panictest: triggered from Ring 3!");
    }
    else if (num == 8) {
        regs->rax = (uint64_t)task::active_user_tasks();
    }
    else if (num == 9) {
        // sys_readdir(size_t index, char* out_name, size_t* out_size)
        regs->rax = vfs::get_entry((size_t)regs->rdi, (char*)regs->rsi, (size_t*)regs->rdx) ? 1 : 0;
    }
    else if (num == 10) {
        // sys_read_file(const char* name, void* buf, size_t max_size) -> bytes read or -1
        regs->rax = (uint64_t)(int64_t)vfs::read((const char*)regs->rdi, (void*)regs->rsi, (size_t)regs->rdx);
    }
    else if (num == 11) {
        // sys_write_file(const char* name, const void* data, size_t size)
        regs->rax = vfs::write((const char*)regs->rdi, (const void*)regs->rsi, (size_t)regs->rdx) ? 1 : 0;
    }
    else if (num == 12) {
        // sys_remove_file(const char* name)
        regs->rax = vfs::remove((const char*)regs->rdi) ? 1 : 0;
    }
    else if (num == 13) {
        console::set_color((uint32_t)regs->rdi);
        regs->rax = 0;
    }
    else if (num == 14) {
        regs->rax = vfs::chdir((const char*)regs->rdi) ? 1 : 0;
    }
    else if (num == 15) {
        // sys_getcwd(char* buf, size_t max_len)
        vfs::getcwd((char*)regs->rdi, (size_t)regs->rsi);
        regs->rax = 0;
    }

    else {
        regs->rax = (uint64_t)-1;
    }
}

void syscall::init() {
    register_syscall_gate(0x80, (void(*)())syscall_entry);
}
