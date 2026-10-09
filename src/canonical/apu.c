#include "apu.h"
#include <string.h>

static const uint8_t length_table[32] = {
    10,254,20,2,40,4,80,6,160,8,60,10,14,12,26,14,
    12,16,24,18,48,20,96,22,192,24,72,26,16,28,32,30
};
static const uint8_t duty_table[4][8] = {
    {0,0,0,0,0,0,0,1}, {0,0,0,0,0,0,1,1},
    {0,0,0,0,1,1,1,1}, {1,1,1,1,1,1,0,0}
};
/* NTSC CPU noise periods are 4, 8, ..., 4068 M2 cycles.
 * The noise divider is clocked at CPU/2; entries are period/2. */
static const uint16_t noise_period_apu[16] = {
    2,4,8,16,32,48,64,80,101,127,190,254,381,508,1017,2034
};

void smb360_apu_init(smb360_apu *apu) {
    memset(apu, 0, sizeof(*apu));
    apu->noise.shift_register=1u; /* NES noise LFSR power-up seed. */
}

/* Pulse 1 has a ones'-complement negative sweep; pulse 2 uses two's complement. */
static int sweep_target(const smb360_apu_pulse *p, unsigned channel) {
    int change=(int)(p->period >> (p->sweep & 7u));
    return (p->sweep & 8u) ? (int)p->period-change-(channel==0u ? 1 : 0)
                             : (int)p->period+change;
}

static uint8_t pulse_output(const smb360_apu_pulse *p, unsigned channel) {
    if (!p->length || p->period<8u || sweep_target(p,channel)>0x7ff ||
        !duty_table[p->control>>6][p->duty_step]) return 0;
    return (p->control & 0x10u) ? (p->control & 15u) : p->envelope_decay;
}

uint8_t smb360_apu_pulse1_output(const smb360_apu *apu) {
    return pulse_output(&apu->pulse1,0u);
}
uint8_t smb360_apu_pulse2_output(const smb360_apu *apu) {
    return pulse_output(&apu->pulse2,1u);
}

/* This is the level on the triangle DAC, not a PCM mixer output.
 * The NES holds this level even while length or linear counter is zero. */
uint8_t smb360_apu_triangle_output(const smb360_apu *apu) {
    unsigned step=apu->triangle.sequence_step;
    return (uint8_t)(step<16u ? 15u-step : step-16u);
}

uint8_t smb360_apu_noise_output(const smb360_apu *apu) {
    const smb360_apu_noise *n=&apu->noise;
    if (!n->length || (n->shift_register&1u)) return 0;
    return (n->control&0x10u) ? (n->control&15u) : n->envelope_decay;
}

static void quarter_noise(smb360_apu_noise *n) {
    if (n->envelope_start) {
        n->envelope_start=0;
        n->envelope_decay=15;
        n->envelope_divider=n->control&15u;
    } else if (!n->envelope_divider) {
        n->envelope_divider=n->control&15u;
        if (n->envelope_decay) --n->envelope_decay;
        else if (n->control&0x20u) n->envelope_decay=15;
    } else --n->envelope_divider;
}

static void half_noise(smb360_apu_noise *n) {
    if (n->length && !(n->control&0x20u)) --n->length;
}

static void quarter_triangle(smb360_apu_triangle *t) {
    if (t->linear_reload_flag) t->linear_counter=t->linear_reload_value;
    else if (t->linear_counter) --t->linear_counter;
    if (!(t->control&0x80u)) t->linear_reload_flag=0;
}

static void half_triangle(smb360_apu_triangle *t) {
    if (t->length && !(t->control&0x80u)) --t->length;
}

static void timer_triangle(smb360_apu_triangle *t) {
    if (t->timer_divider) --t->timer_divider;
    else {
        t->timer_divider=t->period;
        /* At the full CPU clock, neither gating counter resets the phase. */
        if (t->linear_counter && t->length)
            t->sequence_step=(uint8_t)((t->sequence_step+1u)&31u);
    }
}

static void quarter_pulse(smb360_apu_pulse *p) {
    if (p->envelope_start) {
        p->envelope_start=0;
        p->envelope_decay=15;
        p->envelope_divider=p->control&15u;
    } else if (!p->envelope_divider) {
        p->envelope_divider=p->control&15u;
        if (p->envelope_decay) --p->envelope_decay;
        else if (p->control&0x20u) p->envelope_decay=15;
    } else --p->envelope_divider;
}

static void quarter_frame(smb360_apu *apu) {
    ++apu->quarter_clocks;
    quarter_pulse(&apu->pulse1);
    quarter_pulse(&apu->pulse2);
    quarter_triangle(&apu->triangle);
    quarter_noise(&apu->noise);
}

static void half_pulse(smb360_apu_pulse *p, unsigned channel) {
    unsigned sweep_period=(p->sweep>>4)&7u;
    if (p->length && !(p->control&0x20u)) --p->length;
    if (!p->sweep_divider && (p->sweep&0x80u) && (p->sweep&7u) &&
        p->period>=8u && sweep_target(p,channel)<=0x7ff)
        p->period=(uint16_t)sweep_target(p,channel);
    if (!p->sweep_divider || p->sweep_reload) {
        p->sweep_divider=(uint8_t)sweep_period;
        p->sweep_reload=0;
    } else --p->sweep_divider;
}

static void half_frame(smb360_apu *apu) {
    ++apu->half_clocks;
    half_pulse(&apu->pulse1,0u);
    half_pulse(&apu->pulse2,1u);
    half_triangle(&apu->triangle);
    half_noise(&apu->noise);
}

void smb360_apu_write(smb360_apu *apu, uint16_t address, uint8_t value) {
    if (address>=0x4000u && address<=0x4007u) {
        unsigned channel=(unsigned)((address-0x4000u)/4u);
        unsigned reg=(unsigned)((address-0x4000u)%4u);
        smb360_apu_pulse *p=channel==0u?&apu->pulse1:&apu->pulse2;
        switch(reg) {
        case 0: p->control=value; break;
        case 1: p->sweep=value; p->sweep_reload=1; break;
        case 2: p->period=(uint16_t)((p->period&0x700u)|value); break;
        case 3:
            p->period=(uint16_t)((p->period&0xffu)|((uint16_t)(value&7u)<<8));
            if (apu->enabled&(1u<<channel)) p->length=length_table[value>>3];
            p->duty_step=0;
            p->envelope_start=1;
            break;
        default: break;
        }
        return;
    }
    switch(address) {
    case 0x4008u:
        apu->triangle.control=value&0x80u;
        apu->triangle.linear_reload_value=value&0x7fu;
        break;
    case 0x400au:
        apu->triangle.period=(uint16_t)((apu->triangle.period&0x700u)|value);
        break;
    case 0x400bu:
        apu->triangle.period=(uint16_t)((apu->triangle.period&0xffu)|((uint16_t)(value&7u)<<8));
        if (apu->enabled&4u) apu->triangle.length=length_table[value>>3];
        apu->triangle.linear_reload_flag=1u;
        /* Unlike pulse $4003/$4007, $400B does NOT reset phase. */
        break;
    case 0x400cu:
        apu->noise.control=value;
        break;
    case 0x400eu:
        apu->noise.mode=(value>>7)&1u;
        apu->noise.period_index=value&15u;
        break;
    case 0x400fu:
        if (apu->enabled&8u) apu->noise.length=length_table[value>>3];
        apu->noise.envelope_start=1u;
        break;
    case 0x4015u:
        apu->enabled=value&0x1fu;
        if (!(value&1u)) apu->pulse1.length=0;
        if (!(value&2u)) apu->pulse2.length=0;
        if (!(value&4u)) apu->triangle.length=0;
        if (!(value&8u)) apu->noise.length=0;
        break;
    case 0x4017u:
        apu->frame_mode5=(value>>7)&1u;
        apu->irq_inhibit=(value>>6)&1u;
        if (apu->irq_inhibit) apu->frame_irq=0;
        /* APU sequencer reset is deferred to a phi-aligned CPU cycle. */
        apu->frame_reset_delay=(uint8_t)((apu->cpu_cycles&1u)?3u:4u);
        break;
    default: break; /* Other channels are not implemented yet. */
    }
}

uint8_t smb360_apu_read_status(smb360_apu *apu) {
    uint8_t result=(uint8_t)((apu->pulse1.length?1u:0u) |
                              (apu->pulse2.length?2u:0u) |
                              (apu->triangle.length?4u:0u) |
                              (apu->noise.length?8u:0u) |
                              (apu->frame_irq?0x40u:0u));
    apu->frame_irq=0;
    return result;
}

static void timer_pulse(smb360_apu_pulse *p) {
    if (p->timer_divider) --p->timer_divider;
    else {
        p->timer_divider=p->period;
        p->duty_step=(uint8_t)((p->duty_step-1u)&7u);
    }
}

/* Feedback bit 0 xor bit 1 (long mode), or bit 6 (short mode);
 * shift always runs, even when the channel is disabled or its DAC muted. */
static void timer_noise(smb360_apu_noise *n) {
    if (n->timer_divider) --n->timer_divider;
    else {
        unsigned tap=n->mode ? 6u : 1u;
        uint16_t feedback=(uint16_t)((n->shift_register ^ (n->shift_register>>tap))&1u);
        n->shift_register=(uint16_t)((n->shift_register>>1)|(feedback<<14));
        n->timer_divider=(uint16_t)(noise_period_apu[n->period_index]-1u);
    }
}

void smb360_apu_step(smb360_apu *apu, uint32_t cycles) {
    while(cycles--) {
        ++apu->cpu_cycles;
        timer_triangle(&apu->triangle);
        if (!(apu->cpu_cycles&1u)) {
            timer_pulse(&apu->pulse1);
            timer_pulse(&apu->pulse2);
            timer_noise(&apu->noise);
        }
        if (apu->frame_reset_delay) {
            if (!--apu->frame_reset_delay) {
                apu->frame_cycles=0;
                if (apu->frame_mode5) {quarter_frame(apu);half_frame(apu);}
                continue;
            }
        }
        ++apu->frame_cycles;
        if (apu->frame_cycles==7457u || apu->frame_cycles==14913u ||
            apu->frame_cycles==22371u ||
            (!apu->frame_mode5 && apu->frame_cycles==29829u) ||
            (apu->frame_mode5 && apu->frame_cycles==37281u)) {
            quarter_frame(apu);
            if (apu->frame_cycles==14913u || apu->frame_cycles==29829u ||
                apu->frame_cycles==37281u) half_frame(apu);
        }
        if (!apu->frame_mode5 && apu->frame_cycles==29829u &&
            !apu->irq_inhibit) apu->frame_irq=1;
        if (apu->frame_cycles==(apu->frame_mode5?37282u:29830u))
            apu->frame_cycles=0;
    }
}
