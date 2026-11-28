// include/vfs.hpp

#pragma once
#include <stdint.h>
#include <stddef.h>

namespace vfs {
    constexpr size_t MAX_FILES = 64;
    constexpr size_t MAX_NAME_LEN = 64;
    constexpr size_t MAX_FILE_SIZE = 65536;

    struct File {
        char name[MAX_NAME_LEN];
        uint8_t data[MAX_FILE_SIZE];
        size_t size;
        bool used;
    };

    void init();
    void load_tar(void* archive_addr, size_t archive_size);

    bool write(const char* name, const void* data, size_t size);
    int read(const char* name, void* out_buf, size_t max_size);
    bool remove(const char* name);
    bool get_entry(size_t index, char* out_name, size_t* out_size);
    bool chdir(const char* path);
    void getcwd(char* out_buf, size_t max_len);

}
