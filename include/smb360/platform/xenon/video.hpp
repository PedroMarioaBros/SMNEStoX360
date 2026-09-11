#pragma once
#include <cstddef>
#include <cstdint>

namespace smb360 {
// Conservative first video backend: uses LibXenon's public console_pset API.
// This avoids depending on private framebuffer layout and is intended as a
// correctness/bootstrap path; a faster tiled framebuffer backend can replace it
// after hardware validation.
bool xenon_video_present_rgb888(const std::uint8_t* rgb, std::uint32_t width,
                                std::uint32_t height);
}
