// kernlang/tokenizer.cpp
#include "kernlang_internal.hpp"
#include <console.hpp>

namespace kernlang {
    Tokens tokenize(const char* s, int total) {
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

    void print_token(Tokens& t, int idx) {
        if (idx >= t.count) return;
        const char* p = t.ptr[idx];
        int len = t.len[idx];
        
        if (len > 0 && p[0] == '$') {
            Var* v = find_var(p + 1, len - 1);
            if (v) console::puts(v->value);
            else console::puts("[?]");
        } else {
            for (int i = 0; i < len; i++) console::putchar(p[i]);
        }
    }
}
