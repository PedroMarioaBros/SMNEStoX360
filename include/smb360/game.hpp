#pragma once
#include <array>
#include <cstdint>
#include "smb360/platform.hpp"
namespace smb360 {
class Game {
public:
    static const std::uint32_t Width=256, Height=240, TickHz=60;
    Game();
    void reset();
    void tick(const InputState& input);
    const std::uint8_t* indices() const { return pixels_.data(); }
    std::uint64_t frame() const { return frame_; }
private:
    std::array<std::uint8_t, Width*Height> pixels_{};
    std::uint64_t frame_=0;
};
}
