#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>
#include "smb360/game_module.hpp"

namespace smb360 {

class NathsouCoreApi {
public:
    virtual ~NathsouCoreApi() = default;
    virtual bool initialize(const std::uint8_t* chr, std::size_t chr_size, std::uint32_t sample_rate) = 0;
    virtual void reset_game() = 0;
    virtual void set_controller1(std::uint8_t buttons) = 0;
    virtual void run_frame() = 0;
    virtual const std::uint8_t* rgb_frame() const = 0;
    virtual std::size_t rgb_frame_bytes() const = 0;
    virtual void fill_audio(std::uint8_t* mono_u8, std::size_t bytes) = 0;
    virtual std::size_t state_size() const = 0;
    virtual bool save_state(std::uint8_t* out, std::size_t bytes) const = 0;
    virtual bool load_state(const std::uint8_t* in, std::size_t bytes) = 0;
};

class NathsouGame final : public GameModule {
public:
    explicit NathsouGame(NathsouCoreApi& api, std::uint32_t sample_rate = 48000, bool require_known_rom = true);
    bool load_rom(const std::uint8_t* bytes, std::size_t size) override;
    void reset() override;
    void tick(const InputState& input) override;
    GameVideo video() const override;
    GameAudio audio() const override;
    std::uint64_t regression_hash() const override;
    bool initialized() const { return initialized_; }
    std::size_t state_size() const;
    bool save_state(std::uint8_t* out, std::size_t bytes) const;
    bool load_state(const std::uint8_t* in, std::size_t bytes);

    static std::uint8_t encode_buttons(const InputState& input);

private:
    void convert_audio();
    NathsouCoreApi& api_;
    std::uint32_t sample_rate_;
    bool initialized_ = false;
    bool require_known_rom_ = true;
    std::vector<std::uint8_t> chr_;
    std::vector<std::uint8_t> mono_u8_;
    std::vector<std::int16_t> stereo_s16_;
    std::size_t audio_remainder_ = 0;
};

}
