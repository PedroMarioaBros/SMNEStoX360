#pragma once
#include <cstddef>
#include <cstdint>

namespace smb360 {
bool xenon_storage_first_root(char* out_root, std::size_t capacity);
bool xenon_storage_join(const char* root, const char* filename,
                        char* out_path, std::size_t capacity);
// Read exactly expected_size bytes. Rejects shorter and longer files so the
// caller never silently accepts a truncated/extended ROM image.
bool xenon_storage_read_exact(const char* path, std::uint8_t* out,
                              std::size_t expected_size);
}
