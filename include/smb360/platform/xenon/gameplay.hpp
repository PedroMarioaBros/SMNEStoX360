#pragma once
#include "smb360/nathsou_core.hpp"
#include <cstdint>

namespace smb360 {
struct XenonGameplayStepResult {
    bool game_ok = false;
    bool video_ok = false;
    bool audio_submitted = false;
    bool audio_dropped = false;

    bool fatal_ok() const { return game_ok && video_ok; }
};

// Execute one fully wired SMB frame on Xenon: controller -> game core -> video/audio.
// Audio backpressure is reported as a drop, not a fatal frame failure.
XenonGameplayStepResult xenon_gameplay_step(NathsouGame& game, unsigned controller_port = 0);
}
