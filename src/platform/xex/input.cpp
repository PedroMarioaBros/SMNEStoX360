#include "smb360/platform/xex/input.hpp"
#include <cstdint>
#include <cstring>

namespace {
struct XexGamepad {
    std::uint16_t buttons;
    std::uint8_t left_trigger;
    std::uint8_t right_trigger;
    std::int16_t thumb_lx;
    std::int16_t thumb_ly;
    std::int16_t thumb_rx;
    std::int16_t thumb_ry;
};

struct XexInputState {
    std::uint32_t packet_number;
    XexGamepad gamepad;
};

extern "C" std::uint32_t XamInputGetState(std::uint32_t user_index,
                                           std::uint32_t flags,
                                           XexInputState* state);

constexpr std::uint16_t XINPUT_GAMEPAD_DPAD_UP        = 0x0001;
constexpr std::uint16_t XINPUT_GAMEPAD_DPAD_DOWN      = 0x0002;
constexpr std::uint16_t XINPUT_GAMEPAD_DPAD_LEFT      = 0x0004;
constexpr std::uint16_t XINPUT_GAMEPAD_DPAD_RIGHT     = 0x0008;
constexpr std::uint16_t XINPUT_GAMEPAD_START          = 0x0010;
constexpr std::uint16_t XINPUT_GAMEPAD_BACK           = 0x0020;
constexpr std::uint16_t XINPUT_GAMEPAD_A              = 0x1000;
constexpr std::uint16_t XINPUT_GAMEPAD_B              = 0x2000;
}

namespace smb360 {
PadState xex_poll_pad(std::uint32_t index) {
    XexInputState state{};
    PadState out{};
    if (XamInputGetState(index, 0u, &state) != 0u) return out;

    const std::uint16_t b = state.gamepad.buttons;
    if (b & XINPUT_GAMEPAD_DPAD_UP)    out.buttons |= PadUp;
    if (b & XINPUT_GAMEPAD_DPAD_DOWN)  out.buttons |= PadDown;
    if (b & XINPUT_GAMEPAD_DPAD_LEFT)  out.buttons |= PadLeft;
    if (b & XINPUT_GAMEPAD_DPAD_RIGHT) out.buttons |= PadRight;
    if (b & XINPUT_GAMEPAD_A)          out.buttons |= PadA;
    if (b & XINPUT_GAMEPAD_B)          out.buttons |= PadB;
    if (b & XINPUT_GAMEPAD_START)      out.buttons |= PadStart;
    if (b & XINPUT_GAMEPAD_BACK)       out.buttons |= PadBack;

    out.lx = state.gamepad.thumb_lx;
    out.ly = state.gamepad.thumb_ly;
    out.rx = state.gamepad.thumb_rx;
    out.ry = state.gamepad.thumb_ry;
    out.lt = state.gamepad.left_trigger;
    out.rt = state.gamepad.right_trigger;
    return out;
}
}
