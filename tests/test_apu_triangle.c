#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "../src/canonical/apu.h"

/* Synthetic timing tests: no PCM, no Nintendo ROM or external reference. */
static void triangle_setup(smb360_apu *apu, uint8_t linear_control,
                           uint16_t period, uint8_t length_index) {
    smb360_apu_write(apu,0x4015u,4u);
    smb360_apu_write(apu,0x4008u,linear_control);
    smb360_apu_write(apu,0x400au,(uint8_t)period);
    smb360_apu_write(apu,0x400bu,(uint8_t)((length_index<<3)|(period>>8)));
}

static uint8_t expected_level(unsigned index) {
    index &= 31u;
    return (uint8_t)(index<16u ? 15u-index : index-16u);
}

static void test_full_sequence_and_cpu_rate(void) {
    smb360_apu a;
    unsigned i;
    smb360_apu_init(&a);
    assert(smb360_apu_triangle_output(&a)==15u);
    triangle_setup(&a,0xffu,2u,0u); /* linear hold at 127, length 10 */
    assert((smb360_apu_read_status(&a)&7u)==4u);
    assert(a.triangle.linear_reload_flag==1u);
    assert(a.triangle.linear_counter==0u);
    smb360_apu_step(&a,7457u); /* first quarter-frame loads linear */
    assert(a.triangle.linear_counter==127u);
    assert(a.triangle.linear_reload_flag==1u); /* $4008 bit 7 retains flag */
    assert(a.triangle.sequence_step==0u);
    /* Align the divider, then verify the complete 32-step DAC sequence.
     * At period=2 there are 3 CPU clocks between triangle steps. */
    while(a.triangle.timer_divider) smb360_apu_step(&a,1u);
    for(i=1u;i<=32u;i++) {
        unsigned before=a.pulse1.duty_step;
        smb360_apu_step(&a,1u);
        assert(a.triangle.sequence_step==(i&31u));
        assert(smb360_apu_triangle_output(&a)==expected_level(i));
        assert(a.triangle.timer_divider==2u);
        smb360_apu_step(&a,2u);
        assert(a.triangle.sequence_step==(i&31u));
        /* Pulse counters tick only on even CPU cycles, not each CPU cycle.
         * With untouched pulse period 0, phase may advance here. */
        (void)before;
    }
    assert(a.triangle.sequence_step==0u);
    assert(smb360_apu_triangle_output(&a)==15u);
}

static void test_linear_reload_control_and_no_phase_reset(void) {
    smb360_apu a;
    uint8_t level;
    smb360_apu_init(&a);
    triangle_setup(&a,0x03u,8u,3u); /* ctrl clear, linear 3, length 2 */
    assert(a.triangle.length==2u);
    smb360_apu_step(&a,7457u);
    assert(a.quarter_clocks==1u);
    assert(a.triangle.linear_counter==3u);
    assert(a.triangle.linear_reload_flag==0u);
    smb360_apu_write(&a,0x4008u,0x7fu);
    assert(a.triangle.linear_reload_flag==0u); /* $4008 doesn't reload */
    smb360_apu_step(&a,7456u);
    assert(a.triangle.linear_counter==2u);
    assert(a.triangle.length==1u);
    a.triangle.timer_divider=0u;
    smb360_apu_step(&a,1u);
    assert(a.triangle.sequence_step!=0u);
    level=smb360_apu_triangle_output(&a);
    smb360_apu_write(&a,0x400bu,0x18u);
    assert(a.triangle.sequence_step!=0u);
    assert(smb360_apu_triangle_output(&a)==level); /* $400B never resets phase */
    assert(a.triangle.linear_reload_flag==1u);
    smb360_apu_step(&a,7457u); /* quarter 3 */
    assert(a.triangle.linear_counter==127u);
    assert(a.triangle.linear_reload_flag==0u);
    assert(a.triangle.length==2u); /* $400B reloaded the length */
    smb360_apu_step(&a,7458u); /* half 2 */
    assert(a.triangle.length==1u);
}

static void test_gates_hold_last_level(void) {
    smb360_apu a;
    uint8_t level;
    smb360_apu_init(&a);
    triangle_setup(&a,0x84u,1u,3u);
    smb360_apu_step(&a,7457u);
    assert(a.triangle.linear_counter==4u && a.triangle.length==2u);
    a.triangle.timer_divider=0u;
    smb360_apu_step(&a,1u);
    level=smb360_apu_triangle_output(&a);
    assert(level==14u);
    smb360_apu_write(&a,0x4008u,0x80u); /* retain reload flag, value=0 */
    smb360_apu_step(&a,7456u);
    assert(a.triangle.linear_counter==0u);
    assert(a.triangle.length==2u); /* $4008 bit 7 halts length */
    smb360_apu_step(&a,250u);
    assert(smb360_apu_triangle_output(&a)==level);
    smb360_apu_write(&a,0x4015u,0u);
    assert((smb360_apu_read_status(&a)&4u)==0u);
    assert(a.triangle.length==0u && smb360_apu_triangle_output(&a)==level);
    smb360_apu_write(&a,0x4015u,4u);
    assert(a.triangle.length==0u); /* $4015 enable doesn't load length */
    smb360_apu_write(&a,0x4008u,0x87u);
    smb360_apu_write(&a,0x400bu,0x18u);
    assert(a.triangle.length==2u && a.triangle.linear_reload_flag);
    smb360_apu_write(&a,0x4017u,0xc0u);
    smb360_apu_step(&a,4u); /* five-step immediate quarter/half */
    assert(a.triangle.linear_counter==7u);
    assert(a.triangle.length==2u);
    a.triangle.timer_divider=0u;
    smb360_apu_step(&a,1u);
    assert(smb360_apu_triangle_output(&a)!=level); /* resume same phase */
}

static void test_status_register_and_length_isolation(void) {
    smb360_apu a;
    smb360_apu_init(&a);
    smb360_apu_write(&a,0x4015u,7u);
    smb360_apu_write(&a,0x4000u,0xd9u);
    smb360_apu_write(&a,0x4004u,0xdau);
    smb360_apu_write(&a,0x4002u,8u);
    smb360_apu_write(&a,0x4006u,8u);
    smb360_apu_write(&a,0x4003u,0x18u);
    smb360_apu_write(&a,0x4007u,0x18u);
    smb360_apu_write(&a,0x4008u,0x7fu);
    smb360_apu_write(&a,0x400au,0x08u);
    smb360_apu_write(&a,0x400bu,0x18u);
    assert((smb360_apu_read_status(&a)&7u)==7u);
    smb360_apu_write(&a,0x4015u,3u);
    assert((smb360_apu_read_status(&a)&7u)==3u);
    assert(a.pulse1.length==2u && a.pulse2.length==2u);
    smb360_apu_write(&a,0x400bu,0x18u);
    assert(a.triangle.length==0u); /* disabled high write cannot reload */
    smb360_apu_write(&a,0x4015u,4u);
    assert((smb360_apu_read_status(&a)&7u)==0u);
    smb360_apu_write(&a,0x400bu,0x18u);
    assert((smb360_apu_read_status(&a)&7u)==4u);
    assert(a.triangle.period==8u);
    smb360_apu_write(&a,0x4009u,0xffu); /* unused register */
    assert(a.triangle.period==8u);
}

static void test_length_reaches_zero_without_halt(void) {
    smb360_apu a;
    uint8_t level;
    smb360_apu_init(&a);
    triangle_setup(&a,0x7fu,2u,3u); /* control=0, length=2 */
    smb360_apu_step(&a,7457u);
    assert(a.triangle.linear_counter==127u);
    smb360_apu_step(&a,7456u);
    assert(a.triangle.length==1u && a.triangle.linear_counter==126u);
    smb360_apu_step(&a,14916u);
    assert(a.triangle.length==0u);
    assert(a.triangle.linear_counter==124u);
    level=smb360_apu_triangle_output(&a);
    smb360_apu_step(&a,256u);
    assert(smb360_apu_triangle_output(&a)==level);
    assert((smb360_apu_read_status(&a)&4u)==0u);
}

int main(void) {
    test_full_sequence_and_cpu_rate();
    test_linear_reload_control_and_no_phase_reset();
    test_gates_hold_last_level();
    test_status_register_and_length_isolation();
    test_length_reaches_zero_without_halt();
    puts("APU triangle timer/linear/length/32-step waveform: PASS");
    return 0;
}
