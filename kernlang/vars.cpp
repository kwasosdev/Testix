// kernlang/vars.cpp
#include "kernlang_internal.hpp"

namespace kernlang {
    Var vars[MAX_VARS];

    void reset() {
        for (int i = 0; i < MAX_VARS; i++) {
            vars[i].used = false;
        }
    }

    Var* find_var(const char* name, int len) {
        for (int i = 0; i < MAX_VARS; i++) {
            if (!vars[i].used) continue;
            int j = 0;
            while (j < len && vars[i].name[j] == name[j]) j++;
            if (j == len && vars[i].name[j] == 0) return &vars[i];
        }
        return nullptr;
    }

    Var* create_var(const char* name, int len) {
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
}
