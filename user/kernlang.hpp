// user/kernlang.hpp
#pragma once
#include "syscall.hpp"

namespace kernlang {
    constexpr int MAX_VARS = 32;

    struct Var {
        char name[32];
        char value[128];
        bool used;
    };

    struct Tokens {
        const char* ptr[32];
        int len[32];
        int count;
    };

    static Var vars[MAX_VARS];

    static inline void reset() {
        for (int i = 0; i < MAX_VARS; i++) vars[i].used = false;
    }

    static inline Var* find_var(const char* name, int len) {
        for (int i = 0; i < MAX_VARS; i++) {
            if (!vars[i].used) continue;
            int j = 0;
            while (j < len && vars[i].name[j] == name[j]) j++;
            if (j == len && vars[i].name[j] == 0) return &vars[i];
        }
        return nullptr;
    }

    static inline Var* create_var(const char* name, int len) {
        Var* v = find_var(name, len);
        if (v) return v;
        for (int i = 0; i < MAX_VARS; i++) {
            if (!vars[i].used) {
                vars[i].used = true;
                for (int j = 0; j < len && j < 31; j++) vars[i].name[j] = name[j];
                vars[i].name[len < 31 ? len : 31] = 0;
                vars[i].value[0] = 0;
                return &vars[i];
            }
        }
        return nullptr;
    }

    static inline Tokens tokenize(const char* s, int total) {
        Tokens t;
        t.count = 0;
        int i = 0;
        while (i < total && t.count < 32) {
            while (i < total && (s[i] == ' ' || s[i] == '\t')) i++;
            if (i >= total) break;
            
            if (s[i] == '"') {
                i++;
                int start = i;
                while (i < total && s[i] != '"') i++;
                t.ptr[t.count] = s + start;
                t.len[t.count] = i - start;
                t.count++;
                if (i < total) i++;
            } else {
                int start = i;
                while (i < total && s[i] != ' ' && s[i] != '\t') i++;
                t.ptr[t.count] = s + start;
                t.len[t.count] = i - start;
                t.count++;
            }
        }
        return t;
    }

    static inline void print_token(Tokens& t, int idx) {
        if (idx >= t.count) return;
        const char* p = t.ptr[idx];
        int len = t.len[idx];
        
        if (len > 0 && p[0] == '$') {
            Var* v = find_var(p + 1, len - 1);
            if (v) sys_puts(v->value);
            else sys_puts("[?]");
        } else {
            for (int i = 0; i < len; i++) sys_putchar(p[i]);
        }
    }

    static inline void execute(const char* script) {
        int i = 0;
        while (script[i]) {
            int start = i;
            while (script[i] && script[i] != '\n') i++;
            int len = i - start;
            if (script[i] == '\n') i++;
            
            if (len == 0 || script[start] == '#') continue;
            
            Tokens t = tokenize(script + start, len);
            if (t.count == 0) continue;
            
            const char* cmd = t.ptr[0];
            int c_len = t.len[0];
            
            if (c_len == 5 && cmd[0]=='p' && cmd[1]=='r' && cmd[2]=='i' && cmd[3]=='n' && cmd[4]=='t') {
                for (int k = 1; k < t.count; k++) print_token(t, k);
                sys_puts("\n");
            }
            else if (c_len == 3 && cmd[0]=='v' && cmd[1]=='a' && cmd[2]=='r') {
                if (t.count >= 4) {
                    Var* v = create_var(t.ptr[1], t.len[1]);
                    if (v) {
                        const char* val = t.ptr[3];
                        int vlen = t.len[3];
                        int j = 0;
                        if (vlen > 0 && val[0] == '$') {
                            Var* src = find_var(val + 1, vlen - 1);
                            if (src) {
                                while (src->value[j] && j < 127) {
                                    v->value[j] = src->value[j];
                                    j++;
                                }
                            }
                        } else {
                            while (j < vlen && j < 127) {
                                v->value[j] = val[j];
                                j++;
                            }
                        }
                        v->value[j] = 0;
                    }
                }
            }
            else if (c_len == 5 && cmd[0]=='c' && cmd[1]=='l' && cmd[2]=='e' && cmd[3]=='a' && cmd[4]=='r') {
                sys_clear();
            }
            else if (c_len == 4 && cmd[0]=='h' && cmd[1]=='a' && cmd[2]=='l' && cmd[3]=='t') {
                sys_puts("[kernlang] exit.\n");
                sys_exit();
            }
            else if (c_len == 5 && cmd[0]=='s' && cmd[1]=='l' && cmd[2]=='e' && cmd[3]=='e' && cmd[4]=='p') {
                if (t.count >= 2) {
                    int ms = 0;
                    for (int k = 0; k < t.len[1]; k++) {
                        if (t.ptr[1][k] >= '0' && t.ptr[1][k] <= '9') {
                            ms = ms * 10 + (t.ptr[1][k] - '0');
                        }
                    }
                    sys_delay((uint64_t)ms * 100000ULL);
                }
            }
            else {
                sys_puts("[kernlang] unknown: ");
                for (int k = 0; k < c_len; k++) sys_putchar(cmd[k]);
                sys_puts("\n");
            }
        }
    }
}
