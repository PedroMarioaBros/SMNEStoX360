#include "audio_pcm.h"
#include <string.h>

void smb360_pcm_init(smb360_pcm *pcm) { memset(pcm,0,sizeof(*pcm)); }

/* NESdev APU_Mixer: two nonlinear DAC groups, pulse and triangle/noise/DMC.
 * Do not use a linear sum: $4011 DMC level modulates the other channels. */
double smb360_pcm_mix_levels(uint8_t p1, uint8_t p2, uint8_t triangle,
                             uint8_t noise, uint8_t dmc) {
    unsigned pulse=(unsigned)p1+p2;
    double tnd=(double)triangle/8227.0+(double)noise/12241.0+
               (double)dmc/22638.0;
    double pulse_out=pulse ? 95.88/(8128.0/(double)pulse+100.0) : 0.0;
    double tnd_out=tnd>0.0 ? 159.79/(1.0/tnd+100.0) : 0.0;
    return pulse_out+tnd_out;
}

static int16_t pcm_filter(smb360_pcm *p, double mixed) {
    /* First-order 90Hz and 440Hz DC-blocking high passes followed by
     * 14 kHz low pass, all at 48000Hz. Coefficients are fixed so
     * headless WAV captures are deterministic for an identical build. */
    double hp90=0.9883*(p->hp90_prev_out+mixed-p->hp90_prev_in);
    double hp440=0.9440*(p->hp440_prev_out+hp90-p->hp440_prev_in);
    double filtered=p->lp_prev_out+0.8400*(hp440-p->lp_prev_out);
    double scaled=filtered*30000.0;
    p->hp90_prev_in=mixed; p->hp90_prev_out=hp90;
    p->hp440_prev_in=hp90; p->hp440_prev_out=hp440;
    p->lp_prev_out=filtered;
    if(scaled>32767.0)scaled=32767.0;
    if(scaled< -32768.0)scaled= -32768.0;
    return (int16_t)(scaled>=0.0 ? scaled+0.5 : scaled-0.5);
}

void smb360_pcm_advance(smb360_pcm *p, const smb360_apu *a,
                        uint32_t cpu_cycles, smb360_pcm_sink sink, void *context) {
    p->phase+=(uint64_t)cpu_cycles*SMB360_PCM_RATE;
    while(p->phase>=SMB360_NTSC_CPU_HZ) {
        double level=smb360_pcm_mix_levels(smb360_apu_pulse1_output(a),
                                           smb360_apu_pulse2_output(a),
                                           smb360_apu_triangle_output(a),
                                           smb360_apu_noise_output(a),
                                           smb360_apu_dmc_output(a));
        int16_t sample=pcm_filter(p,level);
        p->phase-=SMB360_NTSC_CPU_HZ;
        ++p->samples;
        if(sink)sink(sample,context);
    }
}
