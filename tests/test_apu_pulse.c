#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "../src/canonical/apu.h"

static void write_pulse(smb360_apu *a, uint8_t control, uint8_t sweep,
                        uint16_t period, uint8_t length_index) {
    smb360_apu_write(a,0x4015u,1u);
    smb360_apu_write(a,0x4000u,control);
    smb360_apu_write(a,0x4001u,sweep);
    smb360_apu_write(a,0x4002u,(uint8_t)period);
    smb360_apu_write(a,0x4003u,(uint8_t)((length_index<<3)|(period>>8)));
}

static void test_duty_timer(void) {
    smb360_apu a;
    static const uint8_t expected[8]={0,9,0,0,0,0,0,0};
    unsigned i;
    smb360_apu_init(&a);
    write_pulse(&a,0x10u|9u,0x08u,8u,0u);
    assert(smb360_apu_read_status(&a)==1u);
    for(i=0;i<8u;i++) {
        assert(smb360_apu_pulse1_output(&a)==expected[i]);
        smb360_apu_step(&a, i==0u?2u:18u);
    }
    smb360_apu_write(&a,0x4015u,0);
    assert(smb360_apu_pulse1_output(&a)==0);
    assert(smb360_apu_read_status(&a)==0);
    smb360_apu_write(&a,0x4003u,0);
    assert(smb360_apu_read_status(&a)==0); /* Disabled length cannot reload. */
}

static void test_envelope_length_and_irq(void) {
    smb360_apu a;
    smb360_apu_init(&a);
    write_pulse(&a,0x00u,0x08u,100u,3u); /* length lookup 2 */
    smb360_apu_step(&a,7456u);
    assert(a.quarter_clocks==0u && a.pulse1.envelope_start);
    smb360_apu_step(&a,1u);
    assert(a.quarter_clocks==1u && a.pulse1.envelope_decay==15u);
    smb360_apu_step(&a,7456u);
    assert(a.quarter_clocks==2u && a.half_clocks==1u);
    assert(a.pulse1.length==1u && a.pulse1.envelope_decay==14u);
    smb360_apu_step(&a,14916u);
    assert(a.quarter_clocks==4u && a.half_clocks==2u);
    assert(a.pulse1.length==0u && a.frame_irq==1u);
    assert(smb360_apu_read_status(&a)==0x40u);
    assert(smb360_apu_read_status(&a)==0u);
    smb360_apu_write(&a,0x4017u,0x40u);
    assert(a.irq_inhibit && !a.frame_irq);
}

static void test_five_step_and_sweep(void) {
    smb360_apu a;
    smb360_apu_init(&a);
    write_pulse(&a,0x30u|0x0fu,0x81u,200u,3u);
    smb360_apu_write(&a,0x4017u,0xc0u);
    smb360_apu_step(&a,4u);
    assert(a.quarter_clocks==1u && a.half_clocks==1u);
    assert(a.pulse1.length==2u); /* Halt bit prevents decrement. */
    assert(a.pulse1.period==300u); /* Sweep increase by 200/2. */
    assert(a.pulse1.envelope_decay==15u);
    smb360_apu_step(&a,37282u);
    assert(a.frame_irq==0u && a.quarter_clocks==5u && a.half_clocks==3u);
    smb360_apu_write(&a,0x4002u,0xffu);
    smb360_apu_write(&a,0x4003u,0x07u); /* period=2047, sweep target overflow */
    assert(smb360_apu_pulse1_output(&a)==0u);
    smb360_apu_write(&a,0x4002u,7u);
    smb360_apu_write(&a,0x4003u,0u);
    assert(smb360_apu_pulse1_output(&a)==0u); /* period < 8 */
}

int main(void) {
    test_duty_timer();
    test_envelope_length_and_irq();
    test_five_step_and_sweep();
    puts("APU pulse 1/frame counter: PASS");
    return 0;
}
