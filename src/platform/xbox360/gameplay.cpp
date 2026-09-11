#include "smb360/platform/xenon/gameplay.hpp"
#include "smb360/input_mapper.hpp"
#include "smb360/platform/xenon/audio.hpp"
#include "smb360/platform/xenon/input_adapter.hpp"
#include "smb360/platform/xenon/video.hpp"

namespace smb360 {
XenonGameplayStepResult xenon_gameplay_step(NathsouGame& game, unsigned controller_port) {
    XenonGameplayStepResult result{};
    if (!game.initialized()) return result;

    const PadState pad = xenon_poll_pad(controller_port);
    game.tick(map_pad_to_nes(pad));
    result.game_ok = true;

    const GameVideo video = game.video();
    if (!video.rgb888 || video.width != 256u || video.height != 240u) return result;
    result.video_ok = xenon_video_present_rgb888(video.rgb888, video.width, video.height);
    if (!result.video_ok) return result;

    const GameAudio audio = game.audio();
    if (audio.frames == 0u) {
        result.audio_submitted = true;
        return result;
    }
    if (!audio.stereo) return result;

    switch (xenon_audio_submit(audio.stereo, audio.frames)) {
        case XenonAudioSubmitResult::Submitted:
            result.audio_submitted = true;
            break;
        case XenonAudioSubmitResult::Backpressure:
            result.audio_dropped = true;
            break;
        case XenonAudioSubmitResult::Invalid:
            result.game_ok = false;
            break;
    }
    return result;
}
}
