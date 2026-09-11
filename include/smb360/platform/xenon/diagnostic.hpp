#pragma once
#include "smb360/platform_api.hpp"
#include <cstddef>
#include <cstdint>

namespace smb360 {
constexpr std::uint32_t XenonDiagnosticWidth = 256u;
constexpr std::uint32_t XenonDiagnosticHeight = 240u;
constexpr std::size_t XenonDiagnosticRgbBytes =
    static_cast<std::size_t>(XenonDiagnosticWidth) * XenonDiagnosticHeight * 3u;
constexpr std::size_t XenonDiagnosticAudioFrames = 800u;

struct XenonDiagnosticState {
    std::uint64_t frame = 0u;
    std::uint32_t audio_phase = 0u;
};

void xenon_diagnostic_render_rgb888(const PadState& pad, std::uint64_t frame,
                                    std::uint8_t* rgb, std::size_t bytes);
std::size_t xenon_diagnostic_render_audio(XenonDiagnosticState& state,
                                          std::int16_t* stereo,
                                          std::size_t capacity_frames);
}
