#pragma once
#include <cstdint>

namespace smb360 {
struct XenonFrameTelemetry {
    std::uint64_t frames = 0;
    std::uint64_t fatal_failures = 0;
    std::uint64_t video_failures = 0;
    std::uint64_t audio_drops = 0;
    std::uint64_t missed_deadlines = 0;
    std::uint64_t consecutive_fatal_failures = 0;
    std::uint64_t max_consecutive_fatal_failures = 0;
    std::uint64_t max_lateness_us = 0;
};

void xenon_telemetry_record_frame(XenonFrameTelemetry& stats, bool fatal_ok,
                                  bool video_ok, bool audio_dropped,
                                  std::uint64_t now_us, std::uint64_t deadline_us);
bool xenon_telemetry_should_report(const XenonFrameTelemetry& stats,
                                   std::uint64_t interval_frames = 300u);
bool xenon_telemetry_watchdog_tripped(const XenonFrameTelemetry& stats,
                                      std::uint64_t failure_limit = 120u);
}
