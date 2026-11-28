// kernel/drivers/fs/vfs.cpp

#include <vfs.hpp>
#include <serial.hpp>

namespace vfs {
    static File files[MAX_FILES];
    static char current_dir[MAX_NAME_LEN] = {0};

    static bool str_eq(const char* a, const char* b) {
        if (!a || !b) return false;
        while (*a && *b) { if (*a != *b) return false; a++; b++; }
        return *a == *b;
    }

    static void str_copy(char* dst, const char* src, size_t max_len) {
        if (!dst || !src || max_len == 0) return;
        size_t i = 0;
        while (src[i] && i + 1 < max_len) {
            dst[i] = src[i];
            i++;
        }
        dst[i] = 0;
    }

    static size_t oct2bin(const char* str, int len) {
        size_t n = 0;
        for (int i = 0; i < len && str[i] >= '0' && str[i] <= '7'; i++) {
            n = (n * 8) + (str[i] - '0');
        }
        return n;
    }

    static void resolve_path(const char* input, char* out, size_t max_len) {
        if (!out || max_len == 0) return;
        *out = 0;
        if (!input) return;

        size_t pos = 0;

        // Если путь относительный (не начинается с '/'), стартуем от current_dir
        if (*input != '/') {
            while (current_dir[pos] && pos + 1 < max_len) {
                out[pos] = current_dir[pos];
                pos++;
            }
            out[pos] = 0;
        }

        // Разбираем входной путь по сегментам между '/'
        size_t i = 0;
        while (input[i]) {
            while (input[i] == '/') i++;
            if (input[i] == 0) break;

            size_t start = i;
            while (input[i] && input[i] != '/') i++;
            size_t seg_len = i - start;

            if (seg_len == 1 && input[start] == '.') {
                continue;
            }

            if (seg_len == 2 && input[start] == '.' && input[start + 1] == '.') {
                while (pos > 0 && out[pos - 1] != '/') pos--;
                if (pos > 0 && out[pos - 1] == '/') pos--;
                out[pos] = 0;
                continue;
            }

            if (pos > 0 && pos + 1 < max_len) {
                out[pos++] = '/';
            }
            for (size_t k = 0; k < seg_len && pos + 1 < max_len; k++) {
                out[pos++] = input[start + k];
            }
            out[pos] = 0;
        }
    }

    bool chdir(const char* path) {
        if (!path || *path == 0) {
            *current_dir = 0;
            return true;
        }

        char candidate[MAX_NAME_LEN];
        resolve_path(path, candidate, MAX_NAME_LEN);

        if (*candidate == 0) {
            *current_dir = 0;
            return true;
        }

        size_t clen = 0;
        while (candidate[clen]) clen++;

        for (size_t i = 0; i < MAX_FILES; i++) {
            if (!files[i].used) continue;
            size_t j = 0;
            while (candidate[j] && files[i].name[j] == candidate[j]) j++;
            if (j == clen && files[i].name[j] == '/') {
                str_copy(current_dir, candidate, MAX_NAME_LEN);
                return true;
            }
        }
        return false;
    }

    void init() {
        for (size_t i = 0; i < MAX_FILES; i++) {
            files[i].used = false;
            files[i].size = 0;
        }
        current_dir[0] = 0;
    }

    void getcwd(char* out_buf, size_t max_len) {
        if (!out_buf || max_len < 2) return;
        out_buf[0] = '/';
        if (current_dir[0] == 0) {
            out_buf[1] = 0;
            return;
        }
        size_t i = 0;
        while (current_dir[i] && i + 2 < max_len) {
            out_buf[i + 1] = current_dir[i];
            i++;
        }
        out_buf[i + 1] = 0;
    }

    bool write(const char* name, const void* data, size_t size) {
        if (!name || name[0] == 0 || size > MAX_FILE_SIZE) return false;

        char full_path[MAX_NAME_LEN];
        resolve_path(name, full_path, MAX_NAME_LEN);
        if (full_path[0] == 0) return false;

        int target = -1;
        for (size_t i = 0; i < MAX_FILES; i++) {
            if (files[i].used && str_eq(files[i].name, full_path)) {
                target = (int)i;
                break;
            }
            if (!files[i].used && target == -1) {
                target = (int)i;
            }
        }
        if (target == -1) return false;

        files[target].used = true;
        str_copy(files[target].name, full_path, MAX_NAME_LEN);
        files[target].size = size;

        const uint8_t* src = (const uint8_t*)data;
        for (size_t i = 0; i < size; i++) {
            files[target].data[i] = src[i];
        }
        return true;
    }

    int read(const char* name, void* out_buf, size_t max_size) {
        if (!name || !out_buf) return -1;

        char full_path[MAX_NAME_LEN];
        resolve_path(name, full_path, MAX_NAME_LEN);

        for (size_t i = 0; i < MAX_FILES; i++) {
            if (files[i].used && str_eq(files[i].name, full_path)) {
                size_t to_copy = (files[i].size < max_size) ? files[i].size : max_size;
                uint8_t* dst = (uint8_t*)out_buf;
                for (size_t j = 0; j < to_copy; j++) {
                    dst[j] = files[i].data[j];
                }
                return (int)to_copy;
            }
        }
        return -1;
    }

    bool remove(const char* name) {
        if (!name) return false;

        char full_path[MAX_NAME_LEN];
        resolve_path(name, full_path, MAX_NAME_LEN);

        for (size_t i = 0; i < MAX_FILES; i++) {
            if (files[i].used && str_eq(files[i].name, full_path)) {
                files[i].used = false;
                files[i].size = 0;
                return true;
            }
        }
        return false;
    }

    bool get_entry(size_t index, char* out_name, size_t* out_size) {
        size_t count = 0;
        for (size_t i = 0; i < MAX_FILES; i++) {
            if (files[i].used) {
                if (count == index) {
                    if (out_name) str_copy(out_name, files[i].name, MAX_NAME_LEN);
                    if (out_size) *out_size = files[i].size;
                    return true;
                }
                count++;
            }
        }
        return false;
    }

    struct TarHeader {
        char name[100];
        char mode[8];
        char uid[8];
        char gid[8];
        char size[12];
        char mtime[12];
        char chksum[8];
        char typeflag;
        char linkname[100];
        char magic[6];
        char version[2];
        char uname[32];
        char gname[32];
        char devmajor[8];
        char devminor[8];
        char prefix[155];
    };

    void load_tar(void* archive_addr, size_t archive_size) {
        if (!archive_addr || archive_size < 512) return;
        uint8_t* ptr = (uint8_t*)archive_addr;
        size_t offset = 0;

        while (offset + 512 <= archive_size) {
            TarHeader* hdr = (TarHeader*)(ptr + offset);
            if (hdr->name[0] == 0) break;

            size_t file_size = oct2bin(hdr->size, 12);
            if (hdr->typeflag == '0' || hdr->typeflag == 0) {
                const char* clean_name = hdr->name;
                if (clean_name[0] == '.' && clean_name[1] == '/') clean_name += 2;
                if (clean_name[0] != 0) {
                    write(clean_name, ptr + offset + 512, file_size);
                }
            }

            size_t aligned_size = (file_size + 511) & ~511ULL;
            offset += 512 + aligned_size;
        }
        serial::puts("Testix: initrd.tar mounted into VFS.\n");
    }
}
