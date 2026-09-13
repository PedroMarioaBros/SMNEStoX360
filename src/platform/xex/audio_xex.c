#include "audio_xex.h"
#include <stdint.h>
#include <stddef.h>
#include "apu.h"

#define XAUDIO_CHANNELS 6u
#define XAUDIO_SAMPLES_PER_FRAME 256u

/* xboxkrnl ordinals 499-501, available through xecorelib's import stubs. */
extern uint32_t XAudioRegisterRenderDriverClient(uint32_t *callback_desc,
                                                 uint32_t *driver);
extern uint32_t XAudioUnregisterRenderDriverClient(uint32_t driver);
extern uint32_t XAudioSubmitRenderDriverFrame(uint32_t driver, void *samples);

static volatile uint32_t g_driver;
static volatile int g_ready;
static uint8_t g_mono[XAUDIO_SAMPLES_PER_FRAME] __attribute__((aligned(128)));
static float g_samples[XAUDIO_CHANNELS * XAUDIO_SAMPLES_PER_FRAME]
    __attribute__((aligned(128)));

static void xex_audio_callback(uint32_t ignored) {
    uint32_t i;
    uint32_t ch;
    (void)ignored;
    if (!g_ready || g_driver == 0u) return;

    apu_fill_buffer(g_mono, XAUDIO_SAMPLES_PER_FRAME);
    for (ch = 0; ch < XAUDIO_CHANNELS; ++ch) {
        for (i = 0; i < XAUDIO_SAMPLES_PER_FRAME; ++i)
            g_samples[ch * XAUDIO_SAMPLES_PER_FRAME + i] = 0.0f;
    }

    /* Native Xbox 360 render-driver buffers are six planar float channels at
     * 48 kHz. Feed SMB mono equally to front-left/front-right; leave center,
     * LFE and rear channels silent. On Xenon's big-endian PPC the in-memory
     * float byte order is already the one the native driver consumes. */
    for (i = 0; i < XAUDIO_SAMPLES_PER_FRAME; ++i) {
        float s = (float)((int)g_mono[i] - 128) * (1.0f / 128.0f);
        g_samples[0u * XAUDIO_SAMPLES_PER_FRAME + i] = s;
        g_samples[1u * XAUDIO_SAMPLES_PER_FRAME + i] = s;
    }

    if (XAudioSubmitRenderDriverFrame(g_driver, g_samples) != 0u)
        g_ready = 0;
}

int smb360_xex_audio_init(void) {
    uint32_t callback_desc[2];
    uint32_t driver = 0;
    uint32_t rc;

    g_ready = 0;
    g_driver = 0u;
    callback_desc[0] = (uint32_t)(uintptr_t)&xex_audio_callback;
    callback_desc[1] = 0u;
    rc = XAudioRegisterRenderDriverClient(callback_desc, &driver);
    if (rc != 0u || driver == 0u) return 0;

    g_driver = driver;
    g_ready = 1;
    return 1;
}

void smb360_xex_audio_shutdown(void) {
    uint32_t driver = g_driver;
    g_ready = 0;
    g_driver = 0u;
    if (driver != 0u)
        (void)XAudioUnregisterRenderDriverClient(driver);
}

int smb360_xex_audio_ready(void) {
    return g_ready != 0;
}
