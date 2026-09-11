#pragma once
#include <cstdint>
namespace smb360 {
void xenon_runtime_init();
std::uint64_t xenon_runtime_monotonic_us();
void xenon_runtime_sleep_us(std::uint32_t usec);
void xenon_runtime_sleep_until_us(std::uint64_t deadline_us);
}
