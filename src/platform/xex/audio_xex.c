#include "audio_xex.h"
#include <stdint.h>
#include <stddef.h>
#include "apu.h"

#define XAUDIO_CHANNELS 6u
#define XAUDIO_SAMPLES_PER_FRAME 256u
#define SMB_AUDIO_PER_GAME_FRAME 800u
#define RING_CAPACITY 4096u
#define RING_MASK (RING_CAPACITY - 1u)

/* xboxkrnl ordinals 499-501, available through xecorelib's import stubs. */
extern uint32_t XAudioRegisterRenderDriverClient(uint32_t *callback_desc,
                                                 uint32_t *driver);
extern uint32_t XAudioUnregisterRenderDriverClient(uint32_t driver);
extern uint32_t XAudioSubmitRenderDriverFrame(uint32_t driver, void *samples);

static volatile uint32_t g_driver;
static volatile int g_ready;
static volatile uint32_t g_read_pos;
static volatile uint32_t g_write_pos;
static uint8_t g_ring[RING_CAPACITY] __attribute__((aligned(128)));
static uint8_t g_produced[SMB_AUDIO_PER_GAME_FRAME] __attribute__((aligned(128)));
static float g_samples[XAUDIO_CHANNELS * XAUDIO_SAMPLES_PER_FRAME]
    __attribute__((aligned(128)));

static uint32_t ring_available(void) {
    return (g_write_pos - g_read_pos) & RING_MASK;
}

static uint32_t ring_free(void) {
    return (RING_CAPACITY - 1u) - ring_available();
}

void smb360_xex_audio_produce_frame(void) {
    uint32_t i;
    uint32_t write;
    if (!g_ready) return;

    /* APU access stays entirely on the 60 Hz game thread. */
    apu_fill_buffer(g_produced, SMB_AUDIO_PER_GAME_FRAME);
    if (ring_free() < SMB_AUDIO_PER_GAME_FRAME) {
        /* Drop the oldest queued audio instead of blocking emulation. */
        uint32_t shortage = SMB_AUDIO_PER_GAME_FRAME - ring_free();
        g_read_pos = (g_read_pos + shortage) & RING_MASK;
        __asm__ volatile("sync" ::: "memory");
    }

    write = g_write_pos;
    for (i = 0; i < SMB_AUDIO_PER_GAME_FRAME; ++i) {
        g_ring[write] = g_produced[i];
        write = (write + 1u) & RING_MASK;
    }
    __asm__ volatile("sync" ::: "memory");
    g_write_pos = write;
}

static void xex_audio_callback(uint32_t ignored) {
    uint32_t i;
    uint32_t ch;
    uint32_t read;
    uint32_t available;
    (void)ignored;
    if (!g_ready || g_driver == 0u) return;

    read = g_read_pos;
    available = ring_available();
    for (ch = 0; ch < XAUDIO_CHANNELS; ++ch) {
        for (i = 0; i < XAUDIO_SAMPLES_PER_FRAME; ++i)
            g_samples[ch * XAUDIO_SAMPLES_PER_FRAME + i] = 0.0f;
    }

    for (i = 0; i < XAUDIO_SAMPLES_PER_FRAME; ++i) {
        uint8_t v = 128u;
        if (i < available) {
            v = g_ring[read];
            read = (read + 1u) & RING_MASK;
        }
        {
            float s = (float)((int)v - 128) * (1.0f / 128.0f);
            g_samples[0u * XAUDIO_SAMPLES_PER_FRAME + i] = s;
            g_samples[1u * XAUDIO_SAMPLES_PER_FRAME + i] = s;
        }
    }
    __asm__ volatile("sync" ::: "memory");
    g_read_pos = read;

    if (XAudioSubmitRenderDriverFrame(g_driver, g_samples) != 0u)
        g_ready = 0;
}

int smb360_xex_audio_init(void) {
    uint32_t callback_desc[2];
    uint32_t driver = 0;
    uint32_t rc;

    g_ready = 0;
    g_driver = 0u;
    g_read_pos = 0u;
    g_write_pos = 0u;
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
    __asm__ volatile("sync" ::: "memory");
    if (driver != 0u)
        (void)XAudioUnregisterRenderDriverClient(driver);
}

int smb360_xex_audio_ready(void) {
    return g_ready != 0;
}
