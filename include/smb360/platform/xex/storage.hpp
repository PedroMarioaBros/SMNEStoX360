#pragma once
#include <cstddef>

namespace smb360 {
// Loads a file through Xbox OS/XAM. Relative title paths are tried first so
// a folder containing default.xex + smb.nes works when launched by Aurora/XeXMenu.
bool xex_storage_read_exact(const char* name, void* data, std::size_t size);
}
