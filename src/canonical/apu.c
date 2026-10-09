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

void smb360_apu_init(smb360_apu *apu) { memset(apu, 0, sizeof(*apu)); }

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
    case 0x4015u:
        apu->enabled=value&0x1fu;
        if (!(value&1u)) apu->pulse1.length=0;
        if (!(value&2u)) apu->pulse2.length=0;
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

void smb360_apu_step(smb360_apu *apu, uint32_t cycles) {
    while(cycles--) {
        ++apu->cpu_cycles;
        if (!(apu->cpu_cycles&1u)) {
            timer_pulse(&apu->pulse1);
            timer_pulse(&apu->pulse2);
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
