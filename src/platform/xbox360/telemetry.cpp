#include "smb360/platform/xenon/telemetry.hpp"

namespace smb360 {
void xenon_telemetry_record_frame(XenonFrameTelemetry& stats, bool fatal_ok,
                                  bool video_ok, bool audio_dropped,
                                  std::uint64_t now_us, std::uint64_t deadline_us) {
    ++stats.frames;
    if (!video_ok) ++stats.video_failures;
    if (audio_dropped) ++stats.audio_drops;

    if (fatal_ok) {
        stats.consecutive_fatal_failures = 0;
    } else {
        ++stats.fatal_failures;
        ++stats.consecutive_fatal_failures;
        if (stats.consecutive_fatal_failures > stats.max_consecutive_fatal_failures)
            stats.max_consecutive_fatal_failures = stats.consecutive_fatal_failures;
    }

    if (now_us > deadline_us) {
        ++stats.missed_deadlines;
        const std::uint64_t late = now_us - deadline_us;
        if (late > stats.max_lateness_us) stats.max_lateness_us = late;
    }
}

bool xenon_telemetry_should_report(const XenonFrameTelemetry& stats,
                                   std::uint64_t interval_frames) {
    return interval_frames != 0u && stats.frames != 0u && (stats.frames % interval_frames) == 0u;
}

bool xenon_telemetry_watchdog_tripped(const XenonFrameTelemetry& stats,
                                      std::uint64_t failure_limit) {
    return failure_limit != 0u && stats.consecutive_fatal_failures >= failure_limit;
}
}
