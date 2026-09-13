#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <xecore/xboxkrnl.h>

#include "cpu.h"
#include "ppu.h"
#include "apu.h"
#include "code.h"
#include "common.h"
#include "video_fb.h"
#include "audio_xex.h"
#include "rom_verify.h"

#define NES_RIGHT  0x80u
#define NES_LEFT   0x40u
#define NES_DOWN   0x20u
#define NES_UP     0x10u
#define NES_START  0x08u
#define NES_SELECT 0x04u
#define NES_B      0x02u
#define NES_A      0x01u

#define XINPUT_DPAD_UP    0x0001u
#define XINPUT_DPAD_DOWN  0x0002u
#define XINPUT_DPAD_LEFT  0x0004u
#define XINPUT_DPAD_RIGHT 0x0008u
#define XINPUT_START      0x0010u
#define XINPUT_BACK       0x0020u
#define XINPUT_A          0x1000u
#define XINPUT_B          0x2000u

#define XENON_TIMEBASE_HZ 49875000ull
#define FRAME_HZ 60ull

typedef struct {
    uint16_t buttons;
    uint8_t left_trigger;
    uint8_t right_trigger;
    int16_t thumb_lx;
    int16_t thumb_ly;
    int16_t thumb_rx;
    int16_t thumb_ry;
} smb360_xinput_gamepad;

typedef struct {
    uint32_t packet_number;
    smb360_xinput_gamepad gamepad;
} smb360_xinput_state;

/* xam.xex ordinal 401, provided by xecorelib's xam import library. */
extern uint32_t XamInputGetState(uint32_t user_index, uint32_t flags,
                                 smb360_xinput_state *state);

static uint64_t read_timebase(void) {
    uint32_t hi0, hi1, lo;
    do {
        __asm__ volatile("mftbu %0" : "=r"(hi0));
        __asm__ volatile("mftb %0" : "=r"(lo));
        __asm__ volatile("mftbu %0" : "=r"(hi1));
    } while (hi0 != hi1);
    return ((uint64_t)hi0 << 32) | lo;
}

static void sleep_until(uint64_t deadline) {
    for (;;) {
        uint64_t now = read_timebase();
        uint64_t remaining;
        uint64_t usec;
        int64_t interval;
        if (now >= deadline) return;
        remaining = deadline - now;
        usec = (remaining * 1000000ull) / XENON_TIMEBASE_HZ;
        /* Let the kernel sleep for the bulk of long waits, but leave the last
         * ~500 us to the timebase loop so frame pacing is not scheduler-bound. */
        if (usec <= 500ull) continue;
        usec -= 500ull;
        interval = -(int64_t)(usec * 10ull); /* relative 100 ns units */
        KeDelayExecutionThread(0u, 0u, &interval);
    }
}

static uint8_t poll_nes_pad(int *request_exit) {
    smb360_xinput_state s;
    uint16_t b;
    uint8_t n = 0;
    memset(&s, 0, sizeof(s));
    *request_exit = 0;
    if (XamInputGetState(0u, 0u, &s) != 0u) return 0;
    b = s.gamepad.buttons;
    if (b & XINPUT_DPAD_RIGHT) n |= NES_RIGHT;
    if (b & XINPUT_DPAD_LEFT)  n |= NES_LEFT;
    if (b & XINPUT_DPAD_DOWN)  n |= NES_DOWN;
    if (b & XINPUT_DPAD_UP)    n |= NES_UP;
    if (b & XINPUT_START)      n |= NES_START;
    if (b & XINPUT_BACK)       n |= NES_SELECT;
    if (b & XINPUT_B)          n |= NES_B;
    if (b & XINPUT_A)          n |= NES_A;
    /* Start+Back together returns cleanly to the launcher instead of forcing
     * the user to power-cycle if a test build needs to be left. */
    if ((b & (XINPUT_START | XINPUT_BACK)) == (XINPUT_START | XINPUT_BACK))
        *request_exit = 1;
    return n;
}

int main(void) {
    struct xex_fb fb;
    const char *rom_path;
    uint64_t deadline;
    uint64_t frame_ticks = XENON_TIMEBASE_HZ / FRAME_HZ;
    uint64_t frame_count = 0;
    int audio_ok;

    memset(&fb, 0, sizeof(fb));
    printf("SMB360 XEX: starting Xbox-OS build\n");

    rom_path = smb360_xex_verified_rom_path();
    if (!rom_path) {
        printf("SMB360 XEX: accepted smb.nes not found or SHA1 mismatch\n");
        return 2;
    }
    printf("SMB360 XEX: owner ROM verified at %s\n", rom_path);

    cpu_init();
    apu_init(48000u);
    if (read_chr_rom((char *)rom_path) != 0) {
        printf("SMB360 XEX: verified ROM CHR load failed\n");
        return 3;
    }
    if (!smb360_xex_fb_open(&fb)) {
        printf("SMB360 XEX: active framebuffer discovery failed\n");
        return 4;
    }

    Start();
    audio_ok = smb360_xex_audio_init();
    printf("SMB360 XEX: core initialized, display=%ux%u format=%s audio=%s\n",
           (unsigned)fb.width, (unsigned)fb.height,
           fb.format_10bit ? "A2R10G10B10" : "A8R8G8B8",
           audio_ok ? "native-xaudio" : "disabled");

    deadline = read_timebase() + frame_ticks;
    for (;;) {
        int request_exit = 0;
        uint8_t pad = poll_nes_pad(&request_exit);
        if (request_exit) {
            printf("SMB360 XEX: clean exit requested\n");
            smb360_xex_audio_shutdown();
            return 0;
        }

        update_controller1(pad);
        next_frame();
        ppu_render();
        apu_step_frame();

        if (!smb360_xex_fb_present_rgb888(&fb, frame,
                                          SCREEN_WIDTH, SCREEN_HEIGHT)) {
            printf("SMB360 XEX: framebuffer present failed\n");
            smb360_xex_audio_shutdown();
            return 5;
        }

        ++frame_count;
        if ((frame_count % 600ull) == 0ull)
            printf("SMB360 XEX: frames=%llu audio=%s\n",
                   (unsigned long long)frame_count,
                   smb360_xex_audio_ready() ? "ok" : "disabled");

        sleep_until(deadline);
        deadline += frame_ticks;
        /* Severe stalls should not cause a burst of catch-up frames. */
        if (read_timebase() > deadline + frame_ticks * 4ull)
            deadline = read_timebase() + frame_ticks;
    }
}
