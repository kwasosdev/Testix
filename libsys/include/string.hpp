// libsys/include/string.hpp

#pragma once
#include <stddef.h>
#include "syscall.hpp"

static inline size_t str_len(const char* s) {
    size_t n = 0;
    while (s && s[n]) n++;
    return n;
}

static inline bool str_equals(const char* a, const char* b) {
    if (!a || !b) return false;
    while (*a && *b) {
        if (*a != *b) return false;
        a++; b++;
    }
    return *a == *b;
}

static inline bool str_starts_with(const char* str, const char* prefix) {
    if (!str || !prefix) return false;
    while (*prefix) {
        if (*str++ != *prefix++) return false;
    }
    return true;
}

static inline void print_number(int n) {
    if (n == 0) { sys_putchar('0'); return; }
    if (n < 0) { sys_putchar('-'); n = -n; }
    char buf[16];
    int k = 0;
    while (n > 0) { buf[k++] = '0' + (n % 10); n /= 10; }
    while (k > 0) sys_putchar(buf[--k]);
}

static inline void normalize_path(const char* cwd, const char* input, char* out, size_t max_len) {
    if (!out || max_len < 2) return;
    out[0] = '/';
    out[1] = 0;
    size_t pos = 1;

    if (!input || *input != '/') {
        if (cwd && *cwd == '/' && cwd[1] != 0) {
            size_t c = 1;
            while (cwd[c] && pos + 1 < max_len) out[pos++] = cwd[c++];
            out[pos] = 0;
        }
    }

    if (!input) return;
    size_t i = 0;
    while (input[i]) {
        while (input[i] == '/') i++;
        if (input[i] == 0) break;

        size_t start = i;
        while (input[i] && input[i] != '/') i++;
        size_t seg_len = i - start;

        if (seg_len == 1 && input[start] == '.') continue;
        if (seg_len == 2 && input[start] == '.' && input[start + 1] == '.') {
            while (pos > 1 && out[pos - 1] != '/') pos--;
            if (pos > 1 && out[pos - 1] == '/') pos--;
            out[pos] = 0;
            continue;
        }

        if (pos > 1 && pos + 1 < max_len) out[pos++] = '/';
        for (size_t k = 0; k < seg_len && pos + 1 < max_len; k++) {
            out[pos++] = input[start + k];
        }
        out[pos] = 0;
    }
}
