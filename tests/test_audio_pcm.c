#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "../src/canonical/audio_pcm.h"

typedef struct {
    uint64_t count;
    int16_t minimum,maximum;
    uint64_t nonzero;
    uint64_t checksum;
} pcm_stats;
static void sample_sink(int16_t sample,void *opaque){
    pcm_stats *s=(pcm_stats*)opaque;
    if(sample<s->minimum)s->minimum=sample;
    if(sample>s->maximum)s->maximum=sample;
    if(sample)s->nonzero++;
    s->checksum=s->checksum*131u+(uint16_t)sample;
    ++s->count;
}
static void test_nonlinearity_and_dac_bounds(void){
    double p=smb360_pcm_mix_levels(15u,0u,0u,0u,0u);
    double p2=smb360_pcm_mix_levels(15u,15u,0u,0u,0u);
    double t=smb360_pcm_mix_levels(0u,0u,15u,0u,0u);
    double d=smb360_pcm_mix_levels(0u,0u,0u,0u,127u);
    double all=smb360_pcm_mix_levels(15u,15u,15u,15u,127u);
    assert(smb360_pcm_mix_levels(0u,0u,0u,0u,0u)==0.0);
    assert(p>0.0 && p2>p && p2<2.0*p);
    assert(t>0.0 && d>0.0 && all>p2 && all<1.0);
    assert(smb360_pcm_mix_levels(15u,0u,0u,0u,0u)==
           smb360_pcm_mix_levels(0u,15u,0u,0u,0u));
    assert(smb360_pcm_mix_levels(0u,0u,15u,15u,127u) <
           smb360_pcm_mix_levels(0u,0u,15u,15u,0u)+d);
    /* DMC affects shared triangle/noise DAC nonlinearly. */
    assert(smb360_pcm_mix_levels(0u,0u,15u,15u,127u) >
           smb360_pcm_mix_levels(0u,0u,0u,0u,127u));
}
static void test_exact_sample_clock_and_determinism(void){
    smb360_apu apu;
    smb360_pcm p,q;
    pcm_stats a={0,32767,-32768,0,0},b={0,32767,-32768,0,0};
    smb360_apu_init(&apu);
    smb360_pcm_init(&p);smb360_pcm_init(&q);
    smb360_pcm_advance(&p,&apu,SMB360_NTSC_CPU_HZ,sample_sink,&a);
    assert(a.count==SMB360_PCM_RATE && p.samples==SMB360_PCM_RATE);
    assert(p.phase==0u && a.nonzero==0u);
    for(unsigned i=0;i<SMB360_NTSC_CPU_HZ/79u;i++)
        smb360_pcm_advance(&q,&apu,79u,sample_sink,&b);
    smb360_pcm_advance(&q,&apu,SMB360_NTSC_CPU_HZ%79u,sample_sink,&b);
    assert(b.count==SMB360_PCM_RATE && q.phase==0u);
    assert(a.checksum==b.checksum);
}
static void test_nonzero_audio_clipping_and_repeatable_filter(void){
    smb360_apu apu;
    smb360_pcm p,q;
    pcm_stats a={0,32767,-32768,0,0},b={0,32767,-32768,0,0};
    smb360_apu_init(&apu);
    smb360_apu_write(&apu,0x4015u,1u);
    smb360_apu_write(&apu,0x4000u,0xdfu); /* duty always high initially */
    smb360_apu_write(&apu,0x4002u,100u);
    smb360_apu_write(&apu,0x4003u,0x18u);
    smb360_pcm_init(&p);smb360_pcm_init(&q);
    smb360_pcm_advance(&p,&apu,SMB360_NTSC_CPU_HZ,sample_sink,&a);
    smb360_pcm_advance(&q,&apu,SMB360_NTSC_CPU_HZ,sample_sink,&b);
    assert(a.count==48000u && a.nonzero>0u);
    assert(a.minimum<0 && a.maximum>0);
    assert(a.minimum> -32768 && a.maximum<32767);
    assert(a.checksum==b.checksum);
    assert(smb360_pcm_mix_levels(0,0,0,0,127u)<1.0);
}
int main(void){
    test_nonlinearity_and_dac_bounds();
    test_exact_sample_clock_and_determinism();
    test_nonzero_audio_clipping_and_repeatable_filter();
    puts("APU nonlinear mixer/48kHz PCM clock/filter: PASS");
    return 0;
}
