#include "smb360/game.hpp"
#include "smb360/input_mapper.hpp"
#include "smb360/platform/xenon/input_adapter.hpp"
#include "smb360/platform/xenon/runtime.hpp"
#include "smb360/platform/xenon/audio.hpp"
#include "smb360/platform/xenon/diagnostic.hpp"
#include "smb360/platform/xenon/storage.hpp"
#include "smb360/platform/xenon/telemetry.hpp"
#include "smb360/platform/xenon/video.hpp"
#include "smb360/smb1_identity.hpp"
#include "smb360/fixed_step.hpp"
#include <array>
#include <cstdio>
#include <cstdint>

namespace {
void run_xenon_diagnostics() {
    static std::array<std::uint8_t, smb360::XenonDiagnosticRgbBytes> frame{};
    static std::array<std::int16_t, smb360::XenonDiagnosticAudioFrames * 2u> audio{};
    smb360::XenonDiagnosticState diagnostic{};
    smb360::FixedStepScheduler pacing(60u);
    pacing.reset(smb360::xenon_runtime_monotonic_us());
    pacing.advance();
    std::printf("SMB360 diagnostic mode: generated video/controller/audio test active\n");
    for (;;) {
        const smb360::PadState pad = smb360::xenon_poll_pad(0u);
        smb360::xenon_diagnostic_render_rgb888(pad, diagnostic.frame, frame.data(), frame.size());
        const bool video_ok = smb360::xenon_video_present_rgb888(
            frame.data(), smb360::XenonDiagnosticWidth, smb360::XenonDiagnosticHeight);
        const std::size_t audio_frames = smb360::xenon_diagnostic_render_audio(
            diagnostic, audio.data(), smb360::XenonDiagnosticAudioFrames);
        const smb360::XenonAudioSubmitResult audio_result =
            smb360::xenon_audio_submit(audio.data(), audio_frames);
        if ((diagnostic.frame % 600u) == 0u) {
            std::printf("SMB360 diagnostic frames=%llu video=%s audio=%s buttons=0x%04x\n",
                        static_cast<unsigned long long>(diagnostic.frame),
                        video_ok ? "ok" : "fail",
                        audio_result == smb360::XenonAudioSubmitResult::Submitted ? "ok" :
                        (audio_result == smb360::XenonAudioSubmitResult::Backpressure ? "drop" : "invalid"),
                        static_cast<unsigned>(pad.buttons));
        }
        const std::uint64_t now = smb360::xenon_runtime_monotonic_us();
        if (!pacing.resync_if_late(now, 250000u)) {
            smb360::xenon_runtime_sleep_until_us(pacing.deadline());
            pacing.advance();
        } else {
            std::printf("SMB360 diagnostic pacing: resynchronized after severe stall\n");
        }
    }
}
}

#ifdef SMB360_WITH_NATHSOU_CORE
#include "smb360/nathsou_core.hpp"
#include "smb360/native_nathsou_core.hpp"
#include "smb360/platform/xenon/gameplay.hpp"
#endif

int main(){
    smb360::xenon_runtime_init();
    smb360::xenon_audio_init();
    std::printf("SMB360 LibXenon bootstrap initialized\n");

    char root[64]{};
    char rom_path[128]{};
    std::array<std::uint8_t, 40976> rom{};
    bool rom_ok = false;
    if (smb360::xenon_storage_first_root(root, sizeof(root)) &&
        smb360::xenon_storage_join(root, "smb.nes", rom_path, sizeof(rom_path))) {
        std::printf("Storage ready; expected owner ROM path: %s\n", rom_path);
        if (smb360::xenon_storage_read_exact(rom_path, rom.data(), rom.size())) {
            const smb360::Smb1KnownVariant variant =
                smb360::identify_known_smb1_rom(rom.data(), rom.size());
            rom_ok = variant != smb360::Smb1KnownVariant::Unknown;
            if (rom_ok) {
                std::printf("Owner ROM verified: %s\n", smb360::smb1_variant_name(variant));
            } else {
                std::printf("ROM file found but identity check failed\n");
            }
        } else {
            std::printf("ROM file missing or wrong size\n");
        }
    } else {
        std::printf("Storage unavailable; gameplay ROM not loaded\n");
    }

#ifdef SMB360_WITH_NATHSOU_CORE
    smb360::NativeNathsouCoreApi native_core;
    smb360::NathsouGame game(native_core, 48000u, true);
    if (!rom_ok || !game.load_rom(rom.data(), rom.size())) {
        std::printf("SMB core/ROM unavailable; entering hardware diagnostic mode\n");
        run_xenon_diagnostics();
    }
    std::printf("SMB gameplay core initialized\n");
    smb360::FixedStepScheduler pacing(60u);
    smb360::XenonFrameTelemetry telemetry{};
    pacing.reset(smb360::xenon_runtime_monotonic_us());
    pacing.advance();
    for (;;) {
        const smb360::XenonGameplayStepResult step = smb360::xenon_gameplay_step(game, 0u);
        const std::uint64_t now = smb360::xenon_runtime_monotonic_us();
        smb360::xenon_telemetry_record_frame(telemetry, step.fatal_ok(), step.video_ok,
                                              step.audio_dropped, now, pacing.deadline());
        if (smb360::xenon_telemetry_should_report(telemetry)) {
            std::printf("SMB360 frames=%llu fatal_failures=%llu video_failures=%llu audio_drops=%llu missed_deadlines=%llu max_late_us=%llu\n",
                        static_cast<unsigned long long>(telemetry.frames),
                        static_cast<unsigned long long>(telemetry.fatal_failures),
                        static_cast<unsigned long long>(telemetry.video_failures),
                        static_cast<unsigned long long>(telemetry.audio_drops),
                        static_cast<unsigned long long>(telemetry.missed_deadlines),
                        static_cast<unsigned long long>(telemetry.max_lateness_us));
        }
        if (smb360::xenon_telemetry_watchdog_tripped(telemetry)) {
            std::printf("SMB360 watchdog: 120 consecutive fatal frame failures; halting for diagnosis\n");
            for (;;) smb360::xenon_runtime_sleep_us(1000000u);
        }
        if (!pacing.resync_if_late(now, 250000u)) {
            smb360::xenon_runtime_sleep_until_us(pacing.deadline());
            pacing.advance();
        } else {
            std::printf("SMB360 pacing: resynchronized after severe stall\n");
        }
    }
#else
    (void)rom_ok;
    std::printf("SMB gameplay core not linked; entering hardware diagnostic mode\n");
    run_xenon_diagnostics();
#endif
}
