#pragma once
#include <cstdint>

namespace smb360 {
// Xbox OS/XEX runtime helpers. Unlike the LibXenon backend these execute
// as a normal title under the Xbox 360 kernel.
std::uint64_t xex_runtime_monotonic_us();
void xex_runtime_sleep_us(std::uint32_t usec);
void xex_runtime_sleep_until_us(std::uint64_t deadline_us);
}
