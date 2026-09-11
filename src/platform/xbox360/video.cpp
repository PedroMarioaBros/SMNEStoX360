#include "smb360/platform/xenon/video.hpp"
#include <console/console.h>

namespace smb360 {
bool xenon_video_present_rgb888(const std::uint8_t* rgb, std::uint32_t width,
                                std::uint32_t height) {
    if (!rgb || width == 0 || height == 0) return false;
    // console_pset operates in drawable pixel coordinates and applies the
    // console driver's overscan offset internally. Keep this bootstrap path
    // 1:1 to avoid relying on private mode/framebuffer state.
    for (std::uint32_t y = 0; y < height; ++y) {
        for (std::uint32_t x = 0; x < width; ++x) {
            const std::size_t i = (static_cast<std::size_t>(y) * width + x) * 3u;
            console_pset(static_cast<int>(x), static_cast<int>(y),
                         rgb[i], rgb[i + 1u], rgb[i + 2u]);
        }
    }
    // LibXenon's console_pset() does not flush the data cache. console_putch()
    // does flush the console framebuffer; carriage return changes no pixels.
    console_putch('\r');
    return true;
}
}
