// arch/x86_64/interrupts/idt.cpp

#include <idt.hpp>
#include <panic.hpp>
#include <pic.hpp>
#include <keyboard.hpp>
#include <types.hpp>
#include <task.hpp>

static IDTEntry idt[256];
static IDTPointer idtp;
extern "C" void load_idt(IDTPointer* ptr);

extern "C" void isr0();  extern "C" void isr1();  extern "C" void isr2();  extern "C" void isr3();
extern "C" void isr4();  extern "C" void isr5();  extern "C" void isr6();  extern "C" void isr7();
extern "C" void isr8();  extern "C" void isr9();  extern "C" void isr10(); extern "C" void isr11();
extern "C" void isr12(); extern "C" void isr13(); extern "C" void isr14(); extern "C" void isr15();
extern "C" void isr16(); extern "C" void isr17(); extern "C" void isr18(); extern "C" void isr19();
extern "C" void isr20(); extern "C" void isr21(); extern "C" void isr22(); extern "C" void isr23();
extern "C" void isr24(); extern "C" void isr25(); extern "C" void isr26(); extern "C" void isr27();
extern "C" void isr28(); extern "C" void isr29(); extern "C" void isr30(); extern "C" void isr31();

extern "C" void irq0();  extern "C" void irq1();  extern "C" void irq2();  extern "C" void irq3();
extern "C" void irq4();  extern "C" void irq5();  extern "C" void irq6();  extern "C" void irq7();
extern "C" void irq8();  extern "C" void irq9();  extern "C" void irq10(); extern "C" void irq11();
extern "C" void irq12(); extern "C" void irq13(); extern "C" void irq14(); extern "C" void irq15();

struct Registers {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
    uint64_t int_no, err_code;
    uint64_t rip, cs, rflags, rsp, ss;
};

extern "C" void isr_handler(Registers* regs) {
    panic_hex("Unhandled CPU Exception!", regs->err_code, regs->rip);
}

extern "C" void irq_handler(Registers* regs) {
    if (regs->int_no == 32) {
        task::schedule_from_irq((uint64_t)regs);
    } else if (regs->int_no == 33) {
        keyboard::handle_interrupt();
    }
    pic::send_eoi(regs->int_no - 32);
}

void idt_init() {
    idtp.limit = sizeof(idt) - 1;
    idtp.base = (uint64_t)&idt;

    register_interrupt_handler(0, isr0);
    register_interrupt_handler(1, isr1);
    register_interrupt_handler(2, isr2);
    register_interrupt_handler(3, isr3);
    register_interrupt_handler(4, isr4);
    register_interrupt_handler(5, isr5);
    register_interrupt_handler(6, isr6);
    register_interrupt_handler(7, isr7);
    register_interrupt_handler(8, isr8);
    register_interrupt_handler(9, isr9);
    register_interrupt_handler(10, isr10);
    register_interrupt_handler(11, isr11);
    register_interrupt_handler(12, isr12);
    register_interrupt_handler(13, isr13);
    register_interrupt_handler(14, isr14);
    register_interrupt_handler(15, isr15);
    register_interrupt_handler(16, isr16);
    register_interrupt_handler(17, isr17);
    register_interrupt_handler(18, isr18);
    register_interrupt_handler(19, isr19);
    register_interrupt_handler(20, isr20);
    register_interrupt_handler(21, isr21);
    register_interrupt_handler(22, isr22);
    register_interrupt_handler(23, isr23);
    register_interrupt_handler(24, isr24);
    register_interrupt_handler(25, isr25);
    register_interrupt_handler(26, isr26);
    register_interrupt_handler(27, isr27);
    register_interrupt_handler(28, isr28);
    register_interrupt_handler(29, isr29);
    register_interrupt_handler(30, isr30);
    register_interrupt_handler(31, isr31);

    register_interrupt_handler(32, irq0);
    register_interrupt_handler(33, irq1);
    register_interrupt_handler(34, irq2);
    register_interrupt_handler(35, irq3);
    register_interrupt_handler(36, irq4);
    register_interrupt_handler(37, irq5);
    register_interrupt_handler(38, irq6);
    register_interrupt_handler(39, irq7);
    register_interrupt_handler(40, irq8);
    register_interrupt_handler(41, irq9);
    register_interrupt_handler(42, irq10);
    register_interrupt_handler(43, irq11);
    register_interrupt_handler(44, irq12);
    register_interrupt_handler(45, irq13);
    register_interrupt_handler(46, irq14);
    register_interrupt_handler(47, irq15);

    for (int i = 48; i < 256; i++) {
        register_interrupt_handler(i, isr0);
    }

    load_idt(&idtp);
}

void register_interrupt_handler(uint8_t n, void (*handler)()) {
    uint64_t addr = (uint64_t)handler;
    idt[n].offset_low = addr & 0xFFFF;
    idt[n].offset_middle = (addr >> 16) & 0xFFFF;
    idt[n].offset_high = (addr >> 32) & 0xFFFFFFFF;
    idt[n].selector = 0x08;
    idt[n].ist = 0;
    idt[n].type_attr = 0x8E;
    idt[n].zero = 0;
}

void register_syscall_gate(uint8_t n, void (*handler)()) {
    uint64_t addr = (uint64_t)handler;
    idt[n].offset_low = addr & 0xFFFF;
    idt[n].offset_middle = (addr >> 16) & 0xFFFF;
    idt[n].offset_high = (addr >> 32) & 0xFFFFFFFF;
    idt[n].selector = 0x08;
    idt[n].ist = 0;
    idt[n].type_attr = 0xEE; // DPL=3: доступен из Ring 3!
    idt[n].zero = 0;
}