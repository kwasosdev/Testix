// kernel/kernlang/kernlang.cpp

#include <kernlang.hpp>
#include <console.hpp>

// --- Утилиты ---
static int str_len(const char* s) { int i = 0; while (s[i]) i++; return i; }

// --- Переменные ---
#define MAX_VARS 32
struct Var {
    char name[32];
    char value[128];
    bool used;
};
static Var vars[MAX_VARS];

static Var* find_var(const char* name, int len) {
    for (int i = 0; i < MAX_VARS; i++) {
        if (!vars[i].used) continue;
        int j = 0;
        while (j < len && vars[i].name[j] == name[j]) j++;
        if (j == len && vars[i].name[j] == 0) return &vars[i];
    }
    return nullptr;
}

static Var* create_var(const char* name, int len) {
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

// --- Токенизация ---
struct Tokens {
    const char* ptr[32];
    int len[32];
    int count;
};

static Tokens tokenize(const char* s, int total) {
    Tokens t; t.count = 0;
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

// --- Вывод токена (обрабатывает $var) ---
static void print_token(Tokens& t, int idx) {
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

void kernlang::reset() {
    for (int i = 0; i < MAX_VARS; i++) vars[i].used = false;
}

// --- Интерпретатор ---
void kernlang::execute(const char* script) {
    int i = 0;
    while (script[i]) {
        int start = i;
        while (script[i] && script[i] != '\n') i++;
        int len = i - start;
        if (script[i] == '\n') i++;
        
        // Пропускаем пустые строки и комментарии
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
                        if (src) { while (src->value[j] && j < 127) { v->value[j] = src->value[j]; j++; } }
                    } else {
                        while (j < vlen && j < 127) { v->value[j] = val[j]; j++; }
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
                    if (t.ptr[1][k] >= '0' && t.ptr[1][k] <= '9') ms = ms * 10 + (t.ptr[1][k] - '0');
                }
                // Исправленный цикл без deprecated volatile++
                for (int w = 0; w < ms * 100000; w++) {
                    asm volatile("nop");
                }
            }
        }
        // НЕИЗВЕСТНАЯ КОМАНДА
        else {
            console::puts("[kernlang] unknown: ");
            for (int k = 0; k < c_len; k++) console::putchar(cmd[k]);
            console::puts("\n");
        }
    }
}