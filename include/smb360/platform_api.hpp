#pragma once
#include <cstddef>
#include <cstdint>
namespace smb360 {
struct PadState { std::uint16_t buttons=0; std::int16_t lx=0,ly=0,rx=0,ry=0; std::uint8_t lt=0,rt=0; };
enum PadButton : std::uint16_t {
  PadUp=1u<<0, PadDown=1u<<1, PadLeft=1u<<2, PadRight=1u<<3,
  PadA=1u<<4, PadB=1u<<5, PadStart=1u<<6, PadBack=1u<<7
};
struct VideoSurface { std::uint32_t* pixels=nullptr; std::uint32_t width=0,height=0,pitch_pixels=0; };
class PlatformApi {
public:
  virtual ~PlatformApi() = default;
  virtual bool init()=0;
  virtual std::uint64_t monotonic_us()=0;
  virtual void sleep_us(std::uint32_t)=0;
  virtual PadState pad(std::uint32_t index)=0;
  virtual VideoSurface begin_frame()=0;
  virtual void end_frame()=0;
  virtual std::size_t audio_write(const std::int16_t* stereo, std::size_t frames)=0;
  virtual void shutdown()=0;
};
}
