#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "../src/canonical/apu.h"

static void setup(smb360_apu *a, unsigned channel, uint8_t control,
                  uint8_t sweep, uint16_t period, uint8_t length_index) {
    uint16_t base=(uint16_t)(0x4000u+4u*channel);
    smb360_apu_write(a,base,control);
    smb360_apu_write(a,(uint16_t)(base+1u),sweep);
    smb360_apu_write(a,(uint16_t)(base+2u),(uint8_t)period);
    smb360_apu_write(a,(uint16_t)(base+3u),
                     (uint8_t)((length_index<<3)|(period>>8)));
}

static void test_independent_timers_and_duty(void) {
    smb360_apu a;
    smb360_apu_init(&a);
    smb360_apu_write(&a,0x4015u,3u);
    setup(&a,0u,0x1bu,0x08u,8u,0u);  /* constant volume 11 */
    setup(&a,1u,0x15u,0x08u,10u,1u); /* constant volume 5 */
    assert(smb360_apu_read_status(&a)==3u);
    assert(smb360_apu_pulse1_output(&a)==0);
    assert(smb360_apu_pulse2_output(&a)==0);
    smb360_apu_step(&a,2u);
    assert(smb360_apu_pulse1_output(&a)==11u);
    assert(smb360_apu_pulse2_output(&a)==5u);
    smb360_apu_step(&a,18u);
    assert(a.pulse1.duty_step==6u);
    assert(a.pulse2.duty_step==7u);
    assert(smb360_apu_pulse1_output(&a)==0u);
    assert(smb360_apu_pulse2_output(&a)==5u);
    smb360_apu_write(&a,0x4007u,0x08u); /* pulse 2 reload only */
    assert(a.pulse2.duty_step==0u && a.pulse2.envelope_start);
    assert(a.pulse1.duty_step==6u && a.pulse1.length==10u);
    assert(a.pulse2.period==10u && a.pulse2.length==254u);
    assert(smb360_apu_pulse2_output(&a)==0u);
}

static void test_status_enable_isolation(void) {
    smb360_apu a;
    smb360_apu_init(&a);
    smb360_apu_write(&a,0x4015u,3u);
    setup(&a,0u,0xdau,0x08u,100u,3u);
    setup(&a,1u,0xdbu,0x08u,100u,3u);
    assert(smb360_apu_pulse1_output(&a)==10u);
    assert(smb360_apu_pulse2_output(&a)==11u);
    assert(smb360_apu_read_status(&a)==3u);
    smb360_apu_write(&a,0x4015u,1u);
    assert(smb360_apu_read_status(&a)==1u);
    assert(a.pulse1.length==2u && a.pulse2.length==0u);
    assert(smb360_apu_pulse2_output(&a)==0u);
    smb360_apu_write(&a,0x4007u,0x18u);
    assert(!a.pulse2.length); /* disabled high write cannot load length */
    smb360_apu_write(&a,0x4015u,3u);
    assert(smb360_apu_read_status(&a)==1u); /* enable alone does not load */
    smb360_apu_write(&a,0x4007u,0x18u);
    assert(smb360_apu_read_status(&a)==3u);
    smb360_apu_write(&a,0x4015u,2u);
    assert(smb360_apu_read_status(&a)==2u);
    assert(a.pulse1.length==0u && a.pulse2.length==2u);
}

static void test_negate_sweep_difference(void) {
    smb360_apu a;
    smb360_apu_init(&a);
    smb360_apu_write(&a,0x4015u,3u);
    setup(&a,0u,0xf9u,0x89u,200u,3u); /* negate 200 - 100 - 1 */
    setup(&a,1u,0xfau,0x89u,200u,3u); /* negate 200 - 100 */
    smb360_apu_write(&a,0x4017u,0xc0u);
    smb360_apu_step(&a,4u);
    assert(a.half_clocks==1u && a.quarter_clocks==1u);
    assert(a.pulse1.period==99u);
    assert(a.pulse2.period==100u);
    assert(a.pulse1.length==2u && a.pulse2.length==2u);
    /* Channel 2 overflow should mute only that channel. */
    smb360_apu_write(&a,0x4005u,0x81u);
    smb360_apu_write(&a,0x4006u,0xffu);
    smb360_apu_write(&a,0x4007u,0x07u); /* 2047, duty phase index 0 */
    assert(smb360_apu_pulse2_output(&a)==0u);
    smb360_apu_write(&a,0x4003u,0x18u); /* restore duty phase index 0 */
    assert(smb360_apu_pulse1_output(&a)==9u);
    smb360_apu_write(&a,0x4006u,0x07u);
    smb360_apu_write(&a,0x4007u,0x00u);
    assert(smb360_apu_pulse2_output(&a)==0u); /* timer below 8 */
    smb360_apu_write(&a,0x4006u,0x08u);
    assert(smb360_apu_pulse2_output(&a)==10u);
}

static void test_envelope_and_lengths_both_channels(void) {
    smb360_apu a;
    smb360_apu_init(&a);
    smb360_apu_write(&a,0x4015u,3u);
    setup(&a,0u,0xc0u,0x08u,200u,3u); /* envelope, length=2 */
    setup(&a,1u,0xe1u,0x08u,200u,3u); /* looping envelope, length halt */
    smb360_apu_step(&a,7457u);
    assert(a.quarter_clocks==1u);
    assert(a.pulse1.envelope_decay==15u);
    assert(a.pulse2.envelope_decay==15u);
    smb360_apu_step(&a,7456u);
    assert(a.half_clocks==1u);
    assert(a.pulse1.length==1u && a.pulse2.length==2u);
    assert(a.pulse1.envelope_decay==14u && a.pulse2.envelope_decay==15u);
    smb360_apu_step(&a,7458u);
    assert(a.quarter_clocks==3u);
    assert(a.pulse1.envelope_decay==13u && a.pulse2.envelope_decay==14u);
    smb360_apu_step(&a,7458u);
    assert(a.half_clocks==2u && a.pulse1.length==0u && a.pulse2.length==2u);
    assert((smb360_apu_read_status(&a)&3u)==2u);
    assert(a.frame_irq==0u); /* status read cleared four-step frame IRQ */
}

int main(void) {
    test_independent_timers_and_duty();
    test_status_enable_isolation();
    test_negate_sweep_difference();
    test_envelope_and_lengths_both_channels();
    puts("APU pulse 2 independence/sweep/envelope/status: PASS");
    return 0;
}
