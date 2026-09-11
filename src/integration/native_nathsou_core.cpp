#include "smb360/native_nathsou_core.hpp"
extern "C" {
#include "cpu.h"
#include "ppu.h"
#include "apu.h"
#include "code.h"
void save_state(uint8_t* dest);
void load_state(uint8_t* state);
}
#include <algorithm>

namespace smb360 {
namespace {
constexpr std::size_t kFrameBytes = 256u * 240u * 3u;
constexpr std::size_t kUpstreamStateBytes =
    static_cast<std::size_t>(RAM_SIZE) + NAMETABLE_SIZE + PALETTE_SIZE + OAM_SIZE;

void reset_runtime_state(const std::uint8_t* chr, std::uint32_t sample_rate) {
    // nathsou/smb relies on zero-initialized globals at process startup.  A
    // console reset happens in-process, so explicitly restore that cold-boot
    // state before invoking the upstream initialization sequence.
    std::fill(ram, ram + RAM_SIZE, 0);
    std::fill(nametable, nametable + NAMETABLE_SIZE, 0);
    std::fill(oam, oam + OAM_SIZE, 0);
    std::fill(palette.u8, palette.u8 + PALETTE_SIZE, 0);

    ppu_v = 0;
    ppu_w = 0;
    ppu_f = 0;
    ppu_ctrl = 0;
    ppu_mask = 0;
    ppu_status = 0;
    oam_addr = 0;
    ppu_scroll_x = 0;
    ppu_scroll_y = 0;
    vram_addr = 0;
    vram_internal_buffer = 0;
    oam_dma = 0;

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

void NativeNathsouCoreApi::reset_game() {
    if (!initialized_) return;
    reset_runtime_state(chr_.data(), sample_rate_);
}
void NativeNathsouCoreApi::set_controller1(std::uint8_t buttons) { if (initialized_) update_controller1(buttons); }
void NativeNathsouCoreApi::run_frame() {
    if (!initialized_) return;
    next_frame();
    ppu_render();
    apu_step_frame();
}
const std::uint8_t* NativeNathsouCoreApi::rgb_frame() const { return initialized_ ? frame : nullptr; }
std::size_t NativeNathsouCoreApi::rgb_frame_bytes() const { return initialized_ ? kFrameBytes : 0; }
void NativeNathsouCoreApi::fill_audio(std::uint8_t* out, std::size_t bytes) {
    if (initialized_ && out && bytes) apu_fill_buffer(out, bytes);
}

std::size_t NativeNathsouCoreApi::state_size() const {
    return initialized_ ? kUpstreamStateBytes : 0u;
}

bool NativeNathsouCoreApi::save_state(std::uint8_t* out, std::size_t bytes) const {
    if (!initialized_ || !out || bytes != kUpstreamStateBytes) return false;
    ::save_state(out);
    return true;
}

bool NativeNathsouCoreApi::load_state(const std::uint8_t* in, std::size_t bytes) {
    if (!initialized_ || !in || bytes != kUpstreamStateBytes) return false;
    // Upstream's load_state signature predates const-correctness; it reads but
    // never writes the supplied state buffer.
    ::load_state(const_cast<std::uint8_t*>(in));
    return true;
}
}
