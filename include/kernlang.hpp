#pragma once

namespace kernlang {
    // Выполняет скрипт (строку с \n)
    void execute(const char* script);
    
    // Очищает все переменные (вызывать перед запуском)
    void reset();
}