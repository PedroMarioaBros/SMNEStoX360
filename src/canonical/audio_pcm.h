#ifndef SMB360_CANONICAL_AUDIO_PCM_H
#define SMB360_CANONICAL_AUDIO_PCM_H

#include <stdint.h>
#include "apu.h"

/* NTSC NES 2A03 CPU frequency; output is mono signed 16-bit 48 kHz. */
#define SMB360_NTSC_CPU_HZ 1789773u
#define SMB360_PCM_RATE 48000u

typedef void (*smb360_pcm_sink)(int16_t sample, void *context);

typedef struct {
    uint64_t phase;
    uint64_t samples;
    double hp90_prev_in, hp90_prev_out;
    double hp440_prev_in, hp440_prev_out;
    double lp_prev_out;
} smb360_pcm;

void smb360_pcm_init(smb360_pcm *pcm);
/* Approximate nonlinear NES analog mixer (unfiltered DC level, 0..1). */
double smb360_pcm_mix_levels(uint8_t p1, uint8_t p2, uint8_t triangle,
                             uint8_t noise, uint8_t dmc);
/* Advance by CPU clocks. Levels are sampled at the end of each instruction:
 * interpolation and mid-instruction register writes are not modeled. */
void smb360_pcm_advance(smb360_pcm *pcm, const smb360_apu *apu,
                        uint32_t cpu_cycles, smb360_pcm_sink sink, void *context);

#endif
