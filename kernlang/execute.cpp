// kernlang/execute.cpp
#include "kernlang_internal.hpp"
#include <console.hpp>

namespace kernlang {
    void execute(const char* script) {
        int i = 0;
        while (script[i]) {
            int start = i;
            while (script[i] && script[i] != '\n') i++;
            int len = i - start;
            if (script[i] == '\n') i++;
            
            if (len == 0) continue;
            if (script[start] == '#') continue;
            
            Tokens t = tokenize(script + start, len);
            if (t.count == 0) continue;
            
            const char* cmd = t.ptr[0];
            int c_len = t.len[0];
            
            // print
            if (c_len == 5 && cmd[0]=='p' && cmd[1]=='r' && cmd[2]=='i' && cmd[3]=='n' && cmd[4]=='t') {
                for (int k = 1; k < t.count; k++) print_token(t, k);
                console::puts("\n");
            }
            // var NAME = VALUE
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
            // clear
            else if (c_len == 5 && cmd[0]=='c' && cmd[1]=='l' && cmd[2]=='e' && cmd[3]=='a' && cmd[4]=='r') {
                console::clear();
            }
            // halt
            else if (c_len == 4 && cmd[0]=='h' && cmd[1]=='a' && cmd[2]=='l' && cmd[3]=='t') {
                console::puts("[kernlang] halt.\n");
                for (;;) asm volatile("hlt");
            }
            // sleep N
            else if (c_len == 5 && cmd[0]=='s' && cmd[1]=='l' && cmd[2]=='e' && cmd[3]=='e' && cmd[4]=='p') {
                if (t.count >= 2) {
                    int ms = 0;
                    for (int k = 0; k < t.len[1]; k++) {
                        if (t.ptr[1][k] >= '0' && t.ptr[1][k] <= '9') {
                            ms = ms * 10 + (t.ptr[1][k] - '0');
                        }
                    }
                    for (int w = 0; w < ms * 100000; w++) {
                        asm volatile("nop");
                    }
                }
            }
            // UNKNOWN
            else {
                console::puts("[kernlang] unknown: ");
                for (int k = 0; k < c_len; k++) console::putchar(cmd[k]);
                console::puts("\n");
            }
        }
    }
}
