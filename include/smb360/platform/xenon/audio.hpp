#pragma once
#include <cstddef>
#include <cstdint>

namespace smb360 {
// LibXenon audio contract: 48 kHz, signed 16-bit stereo PCM, little-endian.
// SMB360 renders 800 stereo frames per 60 Hz video frame at 48 kHz.
constexpr std::size_t XenonAudioMaxFramesPerSubmit = 1024;
constexpr std::size_t XenonAudioBytesPerFrame = 4;

enum class XenonAudioSubmitResult : std::uint8_t {
    Submitted,
    Backpressure,
    Invalid,
};

void xenon_audio_init();
std::size_t xenon_audio_pack_le(const std::int16_t* stereo, std::size_t frames,
                                std::uint8_t* out, std::size_t capacity);
XenonAudioSubmitResult xenon_audio_submit(const std::int16_t* stereo, std::size_t frames);
}
