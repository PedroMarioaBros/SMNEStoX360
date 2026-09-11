#include "smb360/native_nathsou_core.hpp"
extern "C" {
#include "cpu.h"
#include "ppu.h"
#include "apu.h"
#include "code.h"
#include "state.h"
}
#include <algorithm>
#include <cstring>

namespace smb360 {
namespace {
constexpr std::size_t kFrameBytes = 256u * 240u * 3u;

void reset_runtime_state(const std::uint8_t* chr, std::uint32_t sample_rate) {
    std::memset(ram, 0, RAM_SIZE);
    std::memset(nametable, 0, NAMETABLE_SIZE);
    std::memset(oam, 0, OAM_SIZE);
    std::memset(palette.u8, 0, PALETTE_SIZE);
    ppu_v = 0; ppu_w = 0; ppu_f = 0; ppu_ctrl = 0; ppu_mask = 0; ppu_status = 0;
    oam_addr = 0; ppu_scroll_x = 0; ppu_scroll_y = 0; vram_addr = 0;
    vram_internal_buffer = 0; oam_dma = 0;
    cpu_init();
    apu_init(sample_rate);
    ppu_init(const_cast<std::uint8_t*>(chr));
    Start();
}
}

bool NativeNathsouCoreApi::initialize(const std::uint8_t* chr, std::size_t chr_size, std::uint32_t sample_rate) {
    if (!chr || chr_size != chr_.size() || sample_rate == 0) return false;
    std::copy(chr, chr + chr_size, chr_.begin());
    sample_rate_ = sample_rate;
    reset_runtime_state(chr_.data(), sample_rate_);
    initialized_ = true;
    return true;
}
void NativeNathsouCoreApi::reset_game() { if (initialized_) reset_runtime_state(chr_.data(), sample_rate_); }
void NativeNathsouCoreApi::set_controller1(std::uint8_t buttons) { if (initialized_) update_controller1(buttons); }
void NativeNathsouCoreApi::run_frame() { if (initialized_) { next_frame(); ppu_render(); apu_step_frame(); } }
const std::uint8_t* NativeNathsouCoreApi::rgb_frame() const { return initialized_ ? frame : nullptr; }
std::size_t NativeNathsouCoreApi::rgb_frame_bytes() const { return initialized_ ? kFrameBytes : 0; }
void NativeNathsouCoreApi::fill_audio(std::uint8_t* out, std::size_t bytes) { if (initialized_ && out && bytes) apu_fill_buffer(out, bytes); }
std::size_t NativeNathsouCoreApi::state_size() const { return initialized_ ? static_cast<std::size_t>(SAVE_STATE_SIZE) : 0u; }
bool NativeNathsouCoreApi::save_state(std::uint8_t* out, std::size_t bytes) const {
    if (!initialized_ || !out || bytes != static_cast<std::size_t>(SAVE_STATE_SIZE)) return false;
    ::save_state(out); return true;
}
bool NativeNathsouCoreApi::load_state(const std::uint8_t* in, std::size_t bytes) {
    if (!initialized_ || !in || bytes != static_cast<std::size_t>(SAVE_STATE_SIZE)) return false;
    ::load_state(const_cast<std::uint8_t*>(in)); return true;
}
}
