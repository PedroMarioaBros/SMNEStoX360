#include "smb360/platform/xex/runtime.hpp"
#include <xecore/xboxkrnl.h>

namespace smb360 {
namespace {
constexpr std::uint64_t kXenonTimebaseHz = 49875000ull;

static inline std::uint64_t read_timebase() {
    std::uint32_t hi0, lo, hi1;
    do {
        __asm__ volatile("mftbu %0" : "=r"(hi0));
        __asm__ volatile("mftb %0" : "=r"(lo));
        __asm__ volatile("mftbu %0" : "=r"(hi1));
    } while (hi0 != hi1);
    return (static_cast<std::uint64_t>(hi0) << 32) | lo;
}
}

std::uint64_t xex_runtime_monotonic_us() {
    const std::uint64_t ticks = read_timebase();
    const std::uint64_t whole = ticks / kXenonTimebaseHz;
    const std::uint64_t remainder = ticks % kXenonTimebaseHz;
    return whole * 1000000ull + (remainder * 1000000ull) / kXenonTimebaseHz;
}

void xex_runtime_sleep_us(std::uint32_t usec) {
    if (usec == 0u) return;
    // Xbox kernel waits use signed 100 ns units; a negative value is relative.
    std::int64_t interval = -static_cast<std::int64_t>(usec) * 10ll;
    KeDelayExecutionThread(0u, 0u, &interval);
}

void xex_runtime_sleep_until_us(std::uint64_t deadline_us) {
    for (;;) {
        const std::uint64_t now = xex_runtime_monotonic_us();
        if (now >= deadline_us) return;
        std::uint64_t remaining = deadline_us - now;
        if (remaining > 1000000ull) remaining = 1000000ull;
        xex_runtime_sleep_us(static_cast<std::uint32_t>(remaining));
    }
}
}
