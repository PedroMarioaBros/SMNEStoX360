#include "smb360/platform/xenon/storage.hpp"
#include <diskio/ata.h>
#include <libfat/fat.h>
#include <cstring>
#include <cstdio>

extern "C" int bdev_enum(int handle, const char** name);

namespace smb360 {
bool xenon_storage_first_root(char* out_root, std::size_t capacity) {
    if (!out_root || capacity == 0) return false;
    out_root[0] = '\0';
    xenon_ata_init();
    xenon_atapi_init();
    if (!fatInitDefault()) return false;

    const char* name = nullptr;
    const int handle = bdev_enum(-1, &name);
    if (handle < 0 || !name || !*name) return false;

    const std::size_t n = std::strlen(name);
    if (n + 3 > capacity) return false; // name + ":/" + NUL
    std::memcpy(out_root, name, n);
    out_root[n] = ':';
    out_root[n + 1] = '/';
    out_root[n + 2] = '\0';
    return true;
}

bool xenon_storage_join(const char* root, const char* filename,
                        char* out_path, std::size_t capacity) {
    if (!root || !filename || !out_path || capacity == 0) return false;
    const std::size_t r = std::strlen(root);
    const std::size_t f = std::strlen(filename);
    const bool slash = r != 0 && root[r - 1] == '/';
    const std::size_t need = r + (slash ? 0u : 1u) + f + 1u;
    if (need > capacity) { out_path[0] = '\0'; return false; }
    std::memcpy(out_path, root, r);
    std::size_t pos = r;
    if (!slash) out_path[pos++] = '/';
    std::memcpy(out_path + pos, filename, f);
    out_path[pos + f] = '\0';
    return true;
}

bool xenon_storage_read_exact(const char* path, std::uint8_t* out,
                              std::size_t expected_size) {
    if (!path || !out || expected_size == 0) return false;
    std::FILE* f = std::fopen(path, "rb");
    if (!f) return false;
    const std::size_t got = std::fread(out, 1u, expected_size, f);
    const int extra = std::fgetc(f);
    const bool io_error = std::ferror(f) != 0;
    std::fclose(f);
    return !io_error && got == expected_size && extra == EOF;
}

}
