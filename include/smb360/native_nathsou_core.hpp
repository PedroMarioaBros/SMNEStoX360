#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include "smb360/nathsou_core.hpp"
namespace smb360 {
class NativeNathsouCoreApi final : public NathsouCoreApi {
public:
    bool initialize(const std::uint8_t* chr, std::size_t chr_size, std::uint32_t sample_rate) override;
    void reset_game() override;
    void set_controller1(std::uint8_t buttons) override;
    void run_frame() override;
    const std::uint8_t* rgb_frame() const override;
    std::size_t rgb_frame_bytes() const override;
    void fill_audio(std::uint8_t* mono_u8, std::size_t bytes) override;
    std::size_t state_size() const override;
    bool save_state(std::uint8_t* out, std::size_t bytes) const override;
    bool load_state(const std::uint8_t* in, std::size_t bytes) override;
private:
    std::array<std::uint8_t,8192> chr_{};
    std::uint32_t sample_rate_ = 48000;
    bool initialized_ = false;
};
}
