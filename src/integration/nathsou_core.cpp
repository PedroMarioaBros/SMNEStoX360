#include "smb360/nathsou_core.hpp"
#include "smb360/ines.hpp"
#include "smb360/smb1_rom.hpp"
#include "smb360/smb1_identity.hpp"
#include "smb360/state_hash.hpp"
#include <algorithm>
#include <cstring>

namespace smb360 {
namespace {
constexpr std::size_t kWidth = 256;
constexpr std::size_t kHeight = 240;
constexpr std::size_t kRgbBytes = kWidth * kHeight * 3;
}

NathsouGame::NathsouGame(NathsouCoreApi& api, std::uint32_t sample_rate, bool require_known_rom)
    : api_(api), sample_rate_(sample_rate), require_known_rom_(require_known_rom) {
    // Reserve once so the 60 Hz audio path does not need to grow its buffers
    // after gameplay starts. The actual frame count is still computed exactly.
    const std::size_t max_frames = (static_cast<std::size_t>(sample_rate_) + 59u) / 60u;
    mono_u8_.reserve(max_frames);
    stereo_s16_.reserve(max_frames * 2u);
}

bool NathsouGame::load_rom(const std::uint8_t* bytes, std::size_t size) {
    initialized_ = false;
    chr_.clear(); mono_u8_.clear(); stereo_s16_.clear(); audio_remainder_ = 0;
    if (!bytes || validate_smb1_rom_layout(bytes, size) != Smb1RomStatus::Ok) return false;
    if (require_known_rom_ && !is_known_smb1_rom(bytes, size)) return false;
    const INesInfo info = parse_ines(bytes, size);
    if (!info.valid || info.chr_size != 8192 || info.chr_offset > size || info.chr_size > size - info.chr_offset) return false;
    // Static recompilation needs only the CHR graphics payload at runtime; the
    // PRG program is already native code, so retaining a second 40 KiB ROM copy
    // wastes console memory.
    chr_.assign(bytes + info.chr_offset, bytes + info.chr_offset + info.chr_size);
    initialized_ = api_.initialize(chr_.data(), chr_.size(), sample_rate_);
    return initialized_;
}

void NathsouGame::reset() {
    if (initialized_) api_.reset_game();
    audio_remainder_ = 0;
    mono_u8_.clear();
    stereo_s16_.clear();
}

std::uint8_t NathsouGame::encode_buttons(const InputState& i) {
    // NES serial-controller order represented as a bit mask: A, B, Select, Start, Up, Down, Left, Right.
    return static_cast<std::uint8_t>((i.a ? 0x01 : 0) | (i.b ? 0x02 : 0) |
        (i.select ? 0x04 : 0) | (i.start ? 0x08 : 0) | (i.up ? 0x10 : 0) |
        (i.down ? 0x20 : 0) | (i.left ? 0x40 : 0) | (i.right ? 0x80 : 0));
}

void NathsouGame::convert_audio() {
    // Exact average sample count at 60 Hz without cumulative drift.
    const std::size_t total = audio_remainder_ + sample_rate_;
    const std::size_t frames = total / 60;
    audio_remainder_ = total % 60;
    mono_u8_.assign(frames, 128);
    if (frames) api_.fill_audio(mono_u8_.data(), frames);
    stereo_s16_.resize(frames * 2);
    for (std::size_t i = 0; i < frames; ++i) {
        const std::int16_t s = static_cast<std::int16_t>((static_cast<int>(mono_u8_[i]) - 128) << 8);
        stereo_s16_[i * 2] = s;
        stereo_s16_[i * 2 + 1] = s;
    }
}

void NathsouGame::tick(const InputState& input) {
    if (!initialized_) return;
    api_.set_controller1(encode_buttons(input));
    api_.run_frame();
    convert_audio();
}

GameVideo NathsouGame::video() const {
    GameVideo out{};
    if (!initialized_) return out;
    const std::uint8_t* rgb = api_.rgb_frame();
    if (!rgb || api_.rgb_frame_bytes() < kRgbBytes) return out;
    out.rgb888 = rgb;
    out.width = static_cast<std::uint32_t>(kWidth);
    out.height = static_cast<std::uint32_t>(kHeight);
    return out;
}

std::size_t NathsouGame::state_size() const {
    return initialized_ ? api_.state_size() : 0u;
}

bool NathsouGame::save_state(std::uint8_t* out, std::size_t bytes) const {
    return initialized_ && api_.save_state(out, bytes);
}

bool NathsouGame::load_state(const std::uint8_t* in, std::size_t bytes) {
    if (!initialized_ || !api_.load_state(in, bytes)) return false;
    // Never replay pre-load audio after restoring game memory.
    audio_remainder_ = 0;
    mono_u8_.clear();
    stereo_s16_.clear();
    return true;
}

GameAudio NathsouGame::audio() const {
    GameAudio out{};
    if (!stereo_s16_.empty()) {
        out.stereo = stereo_s16_.data();
        out.frames = stereo_s16_.size() / 2;
    }
    return out;
}

std::uint64_t NathsouGame::regression_hash() const {
    std::uint64_t h = 1469598103934665603ull;
    const std::uint8_t init = initialized_ ? 1u : 0u;
    h = fnv1a64(&init, sizeof(init), h);
    h = fnv1a64(&audio_remainder_, sizeof(audio_remainder_), h);
    if (initialized_ && api_.rgb_frame() && api_.rgb_frame_bytes() >= kRgbBytes)
        h = fnv1a64(api_.rgb_frame(), kRgbBytes, h);
    if (!stereo_s16_.empty())
        h = fnv1a64(stereo_s16_.data(), stereo_s16_.size() * sizeof(std::int16_t), h);
    return h;
}

}
