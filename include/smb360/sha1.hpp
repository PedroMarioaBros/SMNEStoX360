#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>

namespace smb360 {
using Sha1Digest = std::array<std::uint8_t,20>;
Sha1Digest sha1(const void* data, std::size_t size);
std::string sha1_hex(const void* data, std::size_t size);
}
