// kernel/main.cpp

#include <serial.hpp>
#include <framebuffer.hpp>
#include <limine.hpp>
#include <gdt.hpp>
#include <idt.hpp>
#include <pic.hpp>
#include <keyboard.hpp>
#include <console.hpp>
#include <loader.hpp>
#include <panic.hpp>
#include <tss.hpp>
#include <pmm.hpp>
#include <paging.hpp>
#include <syscall.hpp>
#include <task.hpp>
#include <pit.hpp>
#include <vfs.hpp>

// ===== ЗАПРОСЫ К LIMINE =====

__attribute__((used, section(".limine_requests_start")))
static volatile uint64_t limine_requests_start_marker[4] = LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests")))
static volatile struct limine_base_revision_request base_revision_request = {
    .id = { LIMINE_COMMON_MAGIC, LIMINE_BASE_REVISION_REQUEST_ID },
    .revision = 3
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = { LIMINE_COMMON_MAGIC, LIMINE_FRAMEBUFFER_REQUEST_ID },
    .revision = 0,
    .response = nullptr
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_memmap_request memmap_request = {
    .id = { LIMINE_COMMON_MAGIC, LIMINE_MEMMAP_REQUEST_ID },
    .revision = 0,
    .response = nullptr
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_hhdm_request hhdm_request = {
    .id = { LIMINE_COMMON_MAGIC, LIMINE_HHDM_REQUEST_ID },
    .revision = 0,
    .response = nullptr
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_module_request module_request = {
    .id = { LIMINE_COMMON_MAGIC, LIMINE_MODULE_REQUEST_ID },
    .revision = 0,
    .response = nullptr,
    .internal_module_count = 0,
    .internal_modules = nullptr
};

__attribute__((used, section(".limine_requests_end")))
static volatile uint64_t limine_requests_end_marker[4] = LIMINE_REQUESTS_END_MARKER;

// ===== ТОЧКА ВХОДА =====

extern "C" void kmain(void* boot_info) {
    (void)boot_info;
    serial::init();
    serial::puts("Testix: Booted via Limine!\n");

    if (framebuffer_request.response == nullptr || framebuffer_request.response->framebuffer_count == 0) {
        panic("Limine did not provide a framebuffer!");
    }

    struct limine_framebuffer *lfb = framebuffer_request.response->framebuffers[0];
    
    BootInfo info;
    info.framebuffer_base = (uint64_t)lfb->address;
    info.width = lfb->width;
    info.height = lfb->height;
    info.pixels_per_scanline = lfb->pitch / (lfb->bpp / 8);
    info.bpp = lfb->bpp;

    fb::init(&info);
    console::init(&info);

    console::clear();

    console::puts("Testix OS v0.1\n");
    console::puts("---------------------------------\n\n");

    // 1. Архитектура: IDT первым (чтобы ловить ошибки GDT/TSS)
    serial::puts("Testix: Initializing IDT...\n");
    console::puts("Testix: Initializing IDT...\n");
    idt_init();
    
    serial::puts("Testix: Initializing GDT...\n");
    console::puts("Testix: Initializing GDT...\n");
    gdt_init();
    
    serial::puts("Testix: Initializing TSS...\n");
    console::puts("Testix: Initializing TSS...\n");
    tss::init();

    // 2. Память
    if (memmap_request.response == nullptr) panic("No Memory Map from Limine!");
    serial::puts("Testix: Initializing PMM...\n");
    console::puts("Testix: Initializing PMM...\n");
    pmm::init(memmap_request.response);

    if (hhdm_request.response == nullptr) panic("No HHDM from Limine!");
    paging::set_hhdm_offset(hhdm_request.response->offset);

    // 3. Драйверы и syscall
    serial::puts("Testix: Initializing syscalls...\n");
    console::puts("Testix: Initializing syscalls...\n");
    syscall::init();
    
    serial::puts("Testix: Initializing PIC & Keyboard...\n");
    console::puts("Testix: Initializing PIC & Keyboard...\n");
    pic::remap();
    keyboard::init();

    serial::puts("Testix: Enabling interrupts...\n");
    console::puts("Testix: Enabling interrupts...\n");

    serial::puts("Testix: Enabling pit and task...\n");
    console::puts("Testix: Enabling pit and task...\n\n");
    pit::init(100);
    task::init();
    task::register_current();

    vfs::init();
    if (module_request.response != nullptr && module_request.response->module_count > 0) {
        struct limine_file* mod = module_request.response->modules[0];
        vfs::load_tar(mod->address, mod->size);
    }
    
    console::puts("---------------------------------\n");
    console::puts("Welcome to Testix!\n(c)Kirka 2026\nType \"help\" to see all commands.\n\n");

    asm volatile("cli");
    if (!loader::spawn_file("bin/init")) {
        panic("Failed to load bin/init from initrd!");
    }
    task::sleep_current();
    asm volatile("sti");

    for (;;) {
        asm volatile("hlt");
    }
}