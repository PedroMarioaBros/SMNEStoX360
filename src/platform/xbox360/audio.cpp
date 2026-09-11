#include "smb360/platform/xenon/audio.hpp"
#include <xenon_sound/sound.h>
#include <array>
#include <limits>

namespace smb360 {
namespace {
std::array<std::uint8_t, XenonAudioMaxFramesPerSubmit * XenonAudioBytesPerFrame> g_audio_bytes{};
}

void xenon_audio_init() { xenon_sound_init(); }

std::size_t xenon_audio_pack_le(const std::int16_t* stereo, std::size_t frames,
                                std::uint8_t* out, std::size_t capacity) {
    if (!stereo || !out || frames == 0) return 0;
    if (frames > std::numeric_limits<std::size_t>::max() / XenonAudioBytesPerFrame) return 0;
    const std::size_t bytes = frames * XenonAudioBytesPerFrame;
    if (bytes > capacity) return 0;
    for (std::size_t i = 0; i < frames * 2u; ++i) {
        const std::uint16_t u = static_cast<std::uint16_t>(stereo[i]);
        out[i * 2u] = static_cast<std::uint8_t>(u & 0xffu);
        out[i * 2u + 1u] = static_cast<std::uint8_t>((u >> 8u) & 0xffu);
    }
    return bytes;
}

XenonAudioSubmitResult xenon_audio_submit(const std::int16_t* stereo, std::size_t frames) {
    if (frames == 0) return XenonAudioSubmitResult::Submitted;
    if (!stereo || frames > XenonAudioMaxFramesPerSubmit) return XenonAudioSubmitResult::Invalid;
    const std::size_t packed = xenon_audio_pack_le(stereo, frames, g_audio_bytes.data(), g_audio_bytes.size());
    if (packed == 0 || packed > static_cast<std::size_t>(std::numeric_limits<int>::max())) return XenonAudioSubmitResult::Invalid;
    const int len = static_cast<int>(packed);
    if (xenon_sound_get_free() < len) return XenonAudioSubmitResult::Backpressure;
    xenon_sound_submit(g_audio_bytes.data(), len);
    return XenonAudioSubmitResult::Submitted;
}
}
