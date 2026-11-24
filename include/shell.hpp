#pragma once

namespace shell {
    void init();
    void handle_char(char c);
    void set_editor_mode(bool enable); // Для переключения в редактор
}