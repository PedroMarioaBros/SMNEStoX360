#include "smb360/platform/xex/storage.hpp"
#include <xecore/xam.h>
#include <cstdint>
#include <cstring>

namespace {
constexpr std::uint32_t kGenericRead = 0x80000000u;
constexpr std::uint32_t kFileShareRead = 0x00000001u;
constexpr std::uint32_t kOpenExisting = 3u;

bool read_one(const char* path, void* data, std::size_t size) {
    if (!path || !data || size > 0xffffffffu) return false;
    char writable_path[260]{};
    const std::size_t n = std::strlen(path);
    if (n == 0u || n >= sizeof(writable_path)) return false;
    std::memcpy(writable_path, path, n + 1u);

    HANDLE file = CreateFileA(writable_path, kGenericRead, kFileShareRead,
                              nullptr, kOpenExisting, 0u, nullptr);
    if (file == nullptr || file == reinterpret_cast<HANDLE>(static_cast<std::uintptr_t>(~0u))) {
        return false;
    }
    std::uint32_t got = 0u;
    const bool ok = ReadFile(file, data, static_cast<std::uint32_t>(size), &got, nullptr);
    CloseHandle(file);
    return ok && got == static_cast<std::uint32_t>(size);
}
}

namespace smb360 {
bool xex_storage_read_exact(const char* name, void* data, std::size_t size) {
    if (!name || !*name) return false;
    if (read_one(name, data, size)) return true;

    char game_path[260]{};
    constexpr char prefix[] = "game:\\";
    const std::size_t name_len = std::strlen(name);
    if (sizeof(prefix) - 1u + name_len + 1u > sizeof(game_path)) return false;
    std::memcpy(game_path, prefix, sizeof(prefix) - 1u);
    std::memcpy(game_path + sizeof(prefix) - 1u, name, name_len + 1u);
    return read_one(game_path, data, size);
}
}
