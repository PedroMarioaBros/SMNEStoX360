#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "../src/canonical/apu.h"

/* Synthetic 2A03 noise tests. No PCM or audio fidelity is claimed. */
static const uint16_t ntsc_periods[16]={
    4,8,16,32,64,96,128,160,202,254,380,508,762,1016,2034,4068
};

static void setup(smb360_apu *a,uint8_t control,uint8_t mode_rate,
                  uint8_t length_index) {
    smb360_apu_write(a,0x4015u,8u);
    smb360_apu_write(a,0x400cu,control);
    smb360_apu_write(a,0x400eu,mode_rate);
    smb360_apu_write(a,0x400fu,(uint8_t)(length_index<<3));
}

static void test_reset_timer_and_period_table(void) {
    unsigned i;
    for(i=0u;i<16u;i++) {
        smb360_apu a;
        smb360_apu_init(&a);
        assert(a.noise.shift_register==1u);
        assert(a.noise.timer_divider==0u);
        setup(&a,0x1fu,(uint8_t)i,3u);
        assert(a.noise.length==2u);
        assert(smb360_apu_read_status(&a)==8u);
        assert(smb360_apu_noise_output(&a)==0u); /* LFSR bit 0 set */
        smb360_apu_step(&a,1u);
        assert(a.noise.shift_register==1u); /* CPU odd: APU/2 no clock */
        smb360_apu_step(&a,1u);
        assert(a.noise.shift_register==0x4000u);
        assert(a.noise.timer_divider==ntsc_periods[i]/2u-1u);
        assert(smb360_apu_noise_output(&a)==15u);
        smb360_apu_step(&a,(uint32_t)ntsc_periods[i]-2u);
        assert(a.noise.shift_register==0x4000u);
        assert(a.noise.timer_divider==0u);
        smb360_apu_step(&a,2u);
        assert(a.noise.shift_register==0x2000u);
        assert(a.noise.timer_divider==ntsc_periods[i]/2u-1u);
        assert(a.cpu_cycles==(uint64_t)ntsc_periods[i]+2u);
    }
}

static void test_long_short_mode_taps_and_phase(void) {
    smb360_apu a;
    uint16_t old_shift;
    smb360_apu_init(&a);
    setup(&a,0x1bu,0x00u,0u); /* fixed level 11, length=10 */
    assert(smb360_apu_noise_output(&a)==0u);
    smb360_apu_step(&a,2u);
    assert(a.noise.shift_register==0x4000u);
    assert(smb360_apu_noise_output(&a)==11u);
    /* Deliberately chosen LFSR state distinguishes taps 1 and 6. */
    a.noise.shift_register=0x0041u; /* bit0=1, bit1=0, bit6=1 */
    a.noise.timer_divider=0u;
    smb360_apu_step(&a,2u);
    assert(a.noise.shift_register==0x4020u); /* 1 xor bit1(0) = 1 */
    a.noise.shift_register=0x0041u;
    a.noise.timer_divider=0u;
    smb360_apu_write(&a,0x400eu,0x80u);
    assert(a.noise.shift_register==0x0041u); /* mode write does not reset LFSR */
    smb360_apu_step(&a,2u);
    assert(a.noise.shift_register==0x0020u); /* 1 xor bit6(1) = 0 */
    assert(smb360_apu_noise_output(&a)==11u);
    old_shift=a.noise.shift_register;
    smb360_apu_write(&a,0x400fu,0x18u); /* reset envelope/length, not phase */
    assert(a.noise.shift_register==old_shift);
    a.noise.shift_register=1u;
    assert(smb360_apu_noise_output(&a)==0u);
    smb360_apu_write(&a,0x400du,0xffu); /* unused register */
    assert(a.noise.mode==1u && a.noise.period_index==0u);
}

static void test_envelope_length_and_irq(void) {
    smb360_apu a;
    smb360_apu_init(&a);
    setup(&a,0x00u,0x00u,3u); /* envelope divider 0; length 2 */
    assert(a.noise.envelope_start==1u);
    smb360_apu_step(&a,7456u);
    assert(a.noise.envelope_start==1u && a.quarter_clocks==0u);
    smb360_apu_step(&a,1u);
    assert(a.noise.envelope_start==0u);
    assert(a.noise.envelope_decay==15u);
    smb360_apu_step(&a,7456u);
    assert(a.noise.envelope_decay==14u && a.noise.length==1u);
    smb360_apu_step(&a,7458u);
    assert(a.noise.envelope_decay==13u && a.noise.length==1u);
    smb360_apu_step(&a,7458u);
    assert(a.noise.envelope_decay==12u && a.noise.length==0u);
    assert(smb360_apu_read_status(&a)==0x40u); /* frame IRQ, no noise length */
    assert(smb360_apu_read_status(&a)==0u); /* $4015 read clears frame IRQ */
    assert(smb360_apu_noise_output(&a)==0u);
}

static void test_envelope_loop_and_length_halt(void) {
    smb360_apu a;
    smb360_apu_init(&a);
    setup(&a,0x20u,0x00u,3u); /* envelope loop/halt, envelope period 0 */
    smb360_apu_step(&a,7457u);
    assert(a.noise.envelope_decay==15u && a.noise.length==2u);
    smb360_apu_step(&a,29830u*4u); /* sixteen more quarter frames */
    assert(a.quarter_clocks==17u);
    assert(a.noise.envelope_decay==15u); /* 15->0->15 loops */
    assert(a.noise.length==2u); /* halt flag blocks half-frame decrement */
    smb360_apu_write(&a,0x400cu,0x10u); /* fixed volume 0 and no halt */
    assert(smb360_apu_noise_output(&a)==0u);
    smb360_apu_step(&a,29830u);
    assert(a.noise.length==0u); /* two half-frame decrements */
    assert((smb360_apu_read_status(&a)&8u)==0u);
}

static void test_status_enable_and_channel_isolation(void) {
    smb360_apu a;
    uint16_t shift;
    smb360_apu_init(&a);
    smb360_apu_write(&a,0x4015u,0x0fu);
    smb360_apu_write(&a,0x4000u,0xd9u);
    smb360_apu_write(&a,0x4004u,0xdau);
    smb360_apu_write(&a,0x4002u,8u);
    smb360_apu_write(&a,0x4006u,8u);
    smb360_apu_write(&a,0x4003u,0x18u);
    smb360_apu_write(&a,0x4007u,0x18u);
    smb360_apu_write(&a,0x4008u,0x87u);
    smb360_apu_write(&a,0x400au,8u);
    smb360_apu_write(&a,0x400bu,0x18u);
    smb360_apu_write(&a,0x400cu,0x1bu);
    smb360_apu_write(&a,0x400eu,0x00u);
    smb360_apu_write(&a,0x400fu,0x18u);
    assert((smb360_apu_read_status(&a)&15u)==15u);
    assert(a.pulse1.length==2u && a.pulse2.length==2u);
    assert(a.triangle.length==2u && a.noise.length==2u);
    smb360_apu_step(&a,2u);
    assert(smb360_apu_noise_output(&a)==11u);
    smb360_apu_write(&a,0x4015u,7u);
    assert((smb360_apu_read_status(&a)&15u)==7u);
    assert(a.noise.length==0u && smb360_apu_noise_output(&a)==0u);
    assert(a.pulse1.length==2u && a.pulse2.length==2u && a.triangle.length==2u);
    shift=a.noise.shift_register;
    smb360_apu_step(&a,8u);
    assert(a.noise.shift_register!=shift); /* LFSR clocks even when disabled */
    smb360_apu_write(&a,0x400fu,0x18u);
    assert(a.noise.length==0u); /* disabled high write does not load length */
    assert(a.noise.envelope_start==1u);
    smb360_apu_write(&a,0x4015u,15u);
    assert((smb360_apu_read_status(&a)&15u)==7u); /* enable does not reload */
    smb360_apu_write(&a,0x400fu,0x18u);
    assert((smb360_apu_read_status(&a)&15u)==15u);
    smb360_apu_write(&a,0x4015u,8u);
    assert((smb360_apu_read_status(&a)&15u)==8u);
    assert(a.noise.length==2u && !a.pulse1.length && !a.pulse2.length
           && !a.triangle.length);
}

int main(void) {
    test_reset_timer_and_period_table();
    test_long_short_mode_taps_and_phase();
    test_envelope_length_and_irq();
    test_envelope_loop_and_length_halt();
    test_status_enable_and_channel_isolation();
    puts("APU noise LFSR/timer/envelope/length/status: PASS");
    return 0;
}
