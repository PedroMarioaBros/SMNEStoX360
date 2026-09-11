#include "smb360/platform/xenon/runtime.hpp"
#include <console/console.h>
#include <ppc/timebase.h>
#include <usb/usbmain.h>
#include <xenon_soc/xenon_power.h>
#include <xenos/xenos.h>
#include <time/time.h>
#include <limits>

namespace smb360 {
void xenon_runtime_init() {
    xenos_init(VIDEO_MODE_AUTO);
    console_init();
    xenon_make_it_faster(XENON_SPEED_FULL);
    usb_init();
    usb_do_poll();
    console_clrscr();
}

std::uint64_t xenon_runtime_monotonic_us() {
    const std::uint64_t ticks = mftb();
    const std::uint64_t whole = ticks / static_cast<std::uint64_t>(PPC_TIMEBASE_FREQ);
    const std::uint64_t remainder = ticks % static_cast<std::uint64_t>(PPC_TIMEBASE_FREQ);
    return whole * 1000000ull +
           (remainder * 1000000ull) / static_cast<std::uint64_t>(PPC_TIMEBASE_FREQ);
}

void xenon_runtime_sleep_us(std::uint32_t usec) {
    while (usec != 0u) {
        const std::uint32_t chunk = usec > static_cast<std::uint32_t>(std::numeric_limits<int>::max())
                                      ? static_cast<std::uint32_t>(std::numeric_limits<int>::max())
                                      : usec;
        udelay(static_cast<int>(chunk));
        usec -= chunk;
    }
}

void xenon_runtime_sleep_until_us(std::uint64_t deadline_us) {
    for (;;) {
        const std::uint64_t now = xenon_runtime_monotonic_us();
        if (now >= deadline_us) return;
        const std::uint64_t remaining = deadline_us - now;
        const std::uint32_t chunk = remaining > static_cast<std::uint64_t>(std::numeric_limits<std::uint32_t>::max())
                                      ? std::numeric_limits<std::uint32_t>::max()
                                      : static_cast<std::uint32_t>(remaining);
        xenon_runtime_sleep_us(chunk);
    }
}
}
