#pragma once
#include <cstddef>
#include <cstdint>
namespace smb360 {
struct InputState {
    bool left=false,right=false,up=false,down=false,a=false,b=false,start=false,select=false;
};
class Platform {
public:
    virtual ~Platform() {}
    virtual bool init()=0;
    virtual void poll_input(InputState&)=0;
    virtual void present_rgba(const std::uint32_t*, std::uint32_t, std::uint32_t)=0;
    virtual std::uint64_t ticks_us()=0;
    virtual void sleep_us(std::uint32_t)=0;
    virtual void shutdown()=0;
};
}
