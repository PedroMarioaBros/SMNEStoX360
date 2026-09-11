#pragma once
#include <cstddef>
#include <cstdint>
#include "smb360/platform.hpp"
namespace smb360 {
struct GameVideo {
 const std::uint8_t* indices=nullptr;
 std::uint32_t width=0,height=0;
 const std::uint32_t* palette=nullptr;
 // Optional packed RGB888 path used by licensed static-recompilation cores.
 const std::uint8_t* rgb888=nullptr;
};
struct GameAudio { const std::int16_t* stereo=nullptr; std::size_t frames=0; };
class GameModule {
public:
 virtual ~GameModule()=default;
 virtual bool load_rom(const std::uint8_t* bytes,std::size_t size)=0;
 virtual void reset()=0;
 virtual void tick(const InputState&)=0;
 virtual GameVideo video() const=0;
 virtual GameAudio audio() const=0;
 virtual std::uint64_t regression_hash() const=0;
};
}
