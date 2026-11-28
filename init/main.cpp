// init/main.cpp

#include <syscall.hpp>

extern "C" __attribute__((section(".text._start"))) void _start(const char* args) {
    (void)args;
    sys_puts("[init] starting Testix userspace...\n\n");

    for (;;) {
        int before = sys_active_tasks();
        if (!sys_spawn("bin/shell")) {
            sys_puts("[init] CRITICAL: failed to spawn bin/shell!\n");
            for (;;) sys_yield();
        }

        // Ждём, пока shell (и все его дочерние задачи) не завершатся
        while (sys_active_tasks() > before) {
            sys_yield();
        }

        sys_puts("\n[init] shell exited, respawning...\n");
    }
}
