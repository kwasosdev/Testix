// kernlang/kernlang_internal.hpp
#pragma once
#include <kernlang.hpp>

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

    extern Var vars[MAX_VARS];

    Var* find_var(const char* name, int len);
    Var* create_var(const char* name, int len);
    Tokens tokenize(const char* s, int total);
    void print_token(Tokens& t, int idx);
}
