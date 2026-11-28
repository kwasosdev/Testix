// shell/shell.cpp

#include <console.hpp>
#include <kernlang.hpp>
#include <panic.hpp>
#include <paging.hpp>
#include <pmm.hpp>
#include <syscall.hpp>
#include <framebuffer.hpp>
#include <serial.hpp>
#include <stddef.h>
#include <task.hpp>
#include <loader.hpp>
#include <task.hpp>

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
        console::puts("[shell] spawning user tasks...\n");

        size_t size_a = (size_t)(prog_a_end - prog_a_bin);
        size_t size_b = (size_t)(prog_b_end - prog_b_bin);

        asm volatile("cli");
        loader::spawn_flat(prog_a_bin, size_a);
        loader::spawn_flat(prog_b_bin, size_b);
        task::sleep_current();
        asm volatile("sti");

        // Ждём, пока задачи не разбудят shell через wake_shell()
        while (!task::get_current()->used) {
            asm volatile("hlt");
        }
        console::puts("\n[shell] all user tasks exited.\n");
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
            console::puts("  ring3     - Test Userspace\n");
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