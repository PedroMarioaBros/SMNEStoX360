#include "smb360/platform/xenon/diagnostic.hpp"
#include "smb360/platform_api.hpp"

namespace smb360 {
namespace {
void put_pixel(std::uint8_t* rgb, std::uint32_t x, std::uint32_t y,
               std::uint8_t r, std::uint8_t g, std::uint8_t b) {
    const std::size_t i = (static_cast<std::size_t>(y) * XenonDiagnosticWidth + x) * 3u;
    rgb[i] = r;
    rgb[i + 1u] = g;
    rgb[i + 2u] = b;
}

void fill_rect(std::uint8_t* rgb, std::uint32_t x0, std::uint32_t y0,
               std::uint32_t w, std::uint32_t h, bool active) {
    const std::uint8_t on = active ? 255u : 48u;
    const std::uint8_t off = active ? 48u : 24u;
    for (std::uint32_t y = y0; y < y0 + h && y < XenonDiagnosticHeight; ++y) {
        for (std::uint32_t x = x0; x < x0 + w && x < XenonDiagnosticWidth; ++x) {
            put_pixel(rgb, x, y, on, off, off);
        }
    }
}
}

void xenon_diagnostic_render_rgb888(const PadState& pad, std::uint64_t frame,
                                    std::uint8_t* rgb, std::size_t bytes) {
    if (!rgb || bytes < XenonDiagnosticRgbBytes) return;

    for (std::uint32_t y = 0; y < XenonDiagnosticHeight; ++y) {
        for (std::uint32_t x = 0; x < XenonDiagnosticWidth; ++x) {
            const std::uint32_t bar = (x * 8u) / XenonDiagnosticWidth;
            const std::uint8_t r = (bar & 1u) ? 224u : 32u;
            const std::uint8_t g = (bar & 2u) ? 224u : 32u;
            const std::uint8_t b = (bar & 4u) ? 224u : 32u;
            const bool grid = ((x % 32u) == 0u) || ((y % 30u) == 0u);
            put_pixel(rgb, x, y, grid ? 255u : r, grid ? 255u : g, grid ? 255u : b);
        }
    }

    fill_rect(rgb, 30u, 174u, 14u, 14u, (pad.buttons & PadUp) != 0u);
    fill_rect(rgb, 30u, 204u, 14u, 14u, (pad.buttons & PadDown) != 0u);
    fill_rect(rgb, 15u, 189u, 14u, 14u, (pad.buttons & PadLeft) != 0u);
    fill_rect(rgb, 45u, 189u, 14u, 14u, (pad.buttons & PadRight) != 0u);
    fill_rect(rgb, 186u, 194u, 16u, 16u, (pad.buttons & PadB) != 0u);
    fill_rect(rgb, 214u, 184u, 16u, 16u, (pad.buttons & PadA) != 0u);
    fill_rect(rgb, 104u, 198u, 18u, 8u, (pad.buttons & PadBack) != 0u);
    fill_rect(rgb, 134u, 198u, 18u, 8u, (pad.buttons & PadStart) != 0u);

    const std::uint32_t sweep_x = static_cast<std::uint32_t>(frame % XenonDiagnosticWidth);
    for (std::uint32_t y = 0; y < 8u; ++y) put_pixel(rgb, sweep_x, y, 255u, 255u, 255u);
}

std::size_t xenon_diagnostic_render_audio(XenonDiagnosticState& state,
                                          std::int16_t* stereo,
                                          std::size_t capacity_frames) {
    if (!stereo || capacity_frames < XenonDiagnosticAudioFrames) return 0u;
    // Deterministic 440 Hz square wave at 48 kHz. Keeping the phase in a
    // 0..47999 accumulator makes the generator independent of floating point.
    for (std::size_t i = 0; i < XenonDiagnosticAudioFrames; ++i) {
        const std::int16_t sample = state.audio_phase < 24000u ? 5000 : -5000;
        stereo[i * 2u] = sample;
        stereo[i * 2u + 1u] = sample;
        state.audio_phase += 440u; // one complete cycle per 48000/440 samples.
        if (state.audio_phase >= 48000u) state.audio_phase -= 48000u;
    }
    ++state.frame;
    return XenonDiagnosticAudioFrames;
}
}
