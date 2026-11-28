// binutils/ls.cpp
#include <syscall.hpp>
#include <string.hpp>

constexpr size_t NAME_MAX = 64;
constexpr int MAX_DIRS    = 32;

struct DirEntry {
    char name[NAME_MAX];
};

static bool contains_dir(const DirEntry* dirs, int count, const char* s) {
    for (int i = 0; i < count; i++) {
        if (str_equals(dirs[i].name, s)) return true;
    }
    return false;
}

extern "C" __attribute__((section(".text._start"))) void _start(const char* args) {
    char cwd[NAME_MAX];
    char target[NAME_MAX];
    sys_getcwd(cwd, sizeof(cwd));
    normalize_path(cwd, args, target, sizeof(target));

    int tlen = (int)str_len(target);
    char prefix[NAME_MAX];
    int plen = 0;
    if (tlen > 1) {
        int i = 1;
        while (target[i] && plen < (int)NAME_MAX - 2) prefix[plen++] = target[i++];
        prefix[plen++] = '/';
    }
    prefix[plen] = 0;

    DirEntry shown_dirs[MAX_DIRS];
    int dir_count = 0;

    char fname[NAME_MAX];
    size_t fsize = 0;
    size_t idx = 0;
    bool any = false;

    while (sys_readdir(idx, fname, &fsize)) {
        if (plen == 0 || str_starts_with(fname, prefix)) {
            const char* rel = fname + plen;

            int slash = -1;
            for (int i = 0; rel[i]; i++) {
                if (rel[i] == '/') { slash = i; break; }
            }

            if (slash != -1) {
                char dname[NAME_MAX];
                for (int i = 0; i < slash && i < (int)NAME_MAX - 1; i++) dname[i] = rel[i];
                dname[slash] = 0;

                if (!contains_dir(shown_dirs, dir_count, dname) && dir_count < MAX_DIRS) {
                    int k = 0;
                    while (dname[k]) { shown_dirs[dir_count].name[k] = dname[k]; k++; }
                    shown_dirs[dir_count].name[k] = 0;
                    dir_count++;

                    sys_set_color(0x55AAFF);
                    sys_puts("  ");
                    sys_puts(dname);
                    sys_puts("/\n");
                    sys_set_color(0xFFFFFF);
                    any = true;
                }
            } else if (*rel != 0) {
                bool is_exec = str_starts_with(fname, "bin/");
                sys_puts("  ");
                sys_set_color(is_exec ? 0x55FF55 : 0xFFFFFF);
                sys_puts(rel);
                sys_set_color(0xAAAAAA);
                sys_puts(" (");
                print_number((int)fsize);
                sys_puts(" bytes)\n");
                sys_set_color(0xFFFFFF);
                any = true;
            }
        }
        idx++;
    }

    if (!any) sys_puts("  (empty)\n");
    sys_exit();
}
