#include <console.hpp>
#include <kernlang.hpp>
#include <panic.hpp>
#include <paging.hpp>
#include <pmm.hpp>
#include <syscall.hpp>
#include <framebuffer.hpp>
#include <serial.hpp>
#include <stddef.h>

namespace shell {
    static char cmd_buffer[256];
    static int cmd_len = 0;

    static char script_buf[2048];
    static int script_len = 0;
    static bool editor_mode = false;
    static char edit_line[128];
    static int edit_line_len = 0;
    static int line_count = 0;

    static bool str_equals(const char* a, const char* b) {
        while (*a && *b) { if (*a != *b) return false; a++; b++; }
        return *a == *b;
    }

    static void print_number(int n) {
        if (n == 0) { console::putchar('0'); return; }
        char buf[16];
        int k = 0;
        while (n > 0) { buf[k++] = '0' + (n % 10); n /= 10; }
        while (k > 0) console::putchar(buf[--k]);
    }

    static void script_append_line(const char* line) {
        while (*line && script_len < 2047) script_buf[script_len++] = *line++;
        if (script_len < 2047) script_buf[script_len++] = '\n';
        script_buf[script_len] = 0;
    }

    static void run_script() {
        if (script_len == 0) {
            console::puts("[shell] no script. use 'edit' first.\n");
            return;
        }
        console::puts("[shell] running script...\n");
        console::puts("------------------------------\n");
        kernlang::reset();
        kernlang::execute(script_buf);
        console::puts("------------------------------\n");
        console::puts("[shell] done.\n");
    }

    static void test_ring3() {
        console::puts("[shell] building user program...\n");
        
        static uint8_t user_prog[512];
        size_t user_prog_len = 0;
        auto emit = [&](uint8_t b) { user_prog[user_prog_len++] = b; };
        
        // Генерируем код: sys_write (rax=1) для каждого символа
        const char* msg = "Hello from Ring 3!\n";
        while (*msg) {
            char c = *msg++;
            emit(0x48); emit(0xC7); emit(0xC0); emit(0x01); emit(0); emit(0); emit(0); // mov rax, 1
            emit(0x48); emit(0xC7); emit(0xC7); emit((uint8_t)c); emit(0); emit(0); emit(0); // mov rdi, c
            emit(0xCD); emit(0x80); // int 0x80
        }
        
        // Бесконечный цикл sys_yield (rax=2)
        size_t loop_start = user_prog_len;
        emit(0x48); emit(0xC7); emit(0xC0); emit(0x02); emit(0); emit(0); emit(0);
        emit(0xCD); emit(0x80);
        size_t jmp_end = user_prog_len + 2; // Сохраняем позицию ДО записи jmp
        emit(0xEB); emit((uint8_t)(loop_start - jmp_end));
        
        uint64_t hhdm = paging::get_hhdm_offset();
        uint64_t* user_pml4 = paging::create_address_space();
        
        // ВАЖНО: Мапим Framebuffer для ядра (вычитаем hhdm, чтобы получить физический адрес)
        uint64_t fb_base = (uint64_t)fb::get_base();
        uint32_t fb_size = fb::get_width() * fb::get_height() * (fb::get_bpp() / 8);
        
        uint64_t fb_start = fb_base & ~0xFFF;
        uint64_t fb_end = (fb_base + fb_size + 0xFFF) & ~0xFFF;
        
        for (uint64_t addr = fb_start; addr < fb_end; addr += 4096) {
            paging::map_page(user_pml4, addr, addr - hhdm, PAGE_WRITE);
        }
        
        // Код на 0x400000
        void* code_phys = pmm::alloc_page();
        if (!code_phys) { console::puts("[shell] pmm failed\n"); return; }
        paging::map_page(user_pml4, 0x400000, (uint64_t)code_phys, PAGE_USER | PAGE_WRITE);
        uint8_t* code_virt = (uint8_t*)((uint64_t)code_phys + hhdm);
        for (size_t i = 0; i < user_prog_len; i++) code_virt[i] = user_prog[i];
        
        // Стек на 0x7FFF0000
        void* stack_phys = pmm::alloc_page();
        if (!stack_phys) { console::puts("[shell] pmm failed\n"); return; }
        paging::map_page(user_pml4, 0x7FFF0000, (uint64_t)stack_phys, PAGE_USER | PAGE_WRITE);
        
        paging::switch_address_space(user_pml4);
        
        serial::puts("[shell] entering usermode...\n");
        enter_usermode(0x400000, 0x7FFF0000 + 4096 - 16);
    }

    static void execute_command() {
        if (cmd_len == 0) return;

        console::puts("\n");

        if (str_equals(cmd_buffer, "help")) {
            console::puts("Available commands:\n");
            console::puts("  help      - Show this message\n");
            console::puts("  clear     - Clear screen\n");
            console::puts("  uname     - Show name and version of OS\n");
            console::puts("  panictest - Test Kernel Panic\n");
            console::puts("  edit      - Enter script editor\n");
            console::puts("  list      - Show current script\n");
            console::puts("  run       - Execute current script\n");
            console::puts("  del       - Delete script\n");
            console::puts("  ring3     - Test Userspace (in dev. DO NOT TOUCH!)\n");
        } 
        else if (str_equals(cmd_buffer, "clear")) {
            console::clear();
        } 
        else if (str_equals(cmd_buffer, "uname")) {
            console::puts("Testix OS v0.1\n");
        }
        else if (str_equals(cmd_buffer, "panictest")) {
            panic("panictest: not me!");
        }
        else if (str_equals(cmd_buffer, "edit")) {
            editor_mode = true;
            script_len = 0;
            line_count = 0;
            edit_line_len = 0;
            cmd_len = 0;
            cmd_buffer[0] = 0;
            
            console::puts("[editor] enter lines. '.save' to save, '.cancel' to exit.\n");
            console::puts("1: ");
        }
        else if (str_equals(cmd_buffer, "list")) {
            if (script_len == 0) {
                console::puts("[shell] script is empty.\n");
            } else {
                console::puts("[shell] current script:\n");
                console::puts(script_buf);
            }
        }
        else if (str_equals(cmd_buffer, "run")) {
            run_script();
        }
        else if (str_equals(cmd_buffer, "del")) {
            script_len = 0;
            script_buf[0] = 0;
            console::puts("[shell] script deleted.\n");
        }
        else if (str_equals(cmd_buffer, "ring3")) {
            test_ring3();
        }
        else {
            console::puts("Unknown command: ");
            console::puts(cmd_buffer);
            console::puts("\nType 'help' for commands.\n");
        }

        if (!editor_mode) {
            cmd_len = 0;
            cmd_buffer[0] = 0;
            console::puts("Testix> ");
        }
    }

    void init() {
        cmd_len = 0;
        cmd_buffer[0] = 0;
        console::puts("Testix> ");
    }

    void set_editor_mode(bool enable) {
        editor_mode = enable;
    }

    void handle_char(char c) {
        // ===== РЕЖИМ РЕДАКТОРА =====
        if (editor_mode) {
            if (c == '\n') {
                edit_line[edit_line_len] = 0;
                
                if (str_equals(edit_line, ".save")) {
                    editor_mode = false;
                    cmd_len = 0;
                    cmd_buffer[0] = 0;
                    console::puts("\n[editor] saved ");
                    print_number(line_count);
                    console::puts(" lines.\n");
                    console::puts("Testix> ");
                } 
                else if (str_equals(edit_line, ".cancel")) {
                    editor_mode = false;
                    cmd_len = 0;
                    cmd_buffer[0] = 0;
                    console::puts("\n[editor] cancelled.\n");
                    console::puts("Testix> ");
                } 
                else {
                    script_append_line(edit_line);
                    line_count++;
                    edit_line_len = 0;
                    console::putchar('\n');
                    print_number(line_count + 1);
                    console::puts(": ");
                }
            } 
            else if (c == '\b') {
                if (edit_line_len > 0) {
                    edit_line_len--;
                    console::putchar('\b');
                }
            } 
            else {
                if (edit_line_len < 127) {
                    edit_line[edit_line_len++] = c;
                    console::putchar(c);
                }
            }
            return;
        }
        
        // ===== ОБЫЧНЫЙ РЕЖИМ (SHELL) =====
        if (c == '\n') {
            cmd_buffer[cmd_len] = 0;
            execute_command();
        } 
        else if (c == '\b') {
            if (cmd_len > 0) {
                cmd_len--;
                console::putchar('\b');
            }
        } 
        else {
            if (cmd_len < 255) {
                cmd_buffer[cmd_len] = c;
                cmd_len++;
                console::putchar(c);
            }
        }
    }
}