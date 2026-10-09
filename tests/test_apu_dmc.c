#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "../src/canonical/apu.h"
#include "../src/canonical/machine.h"

static void configure(smb360_apu *a, uint8_t control, uint8_t addr,
                      uint8_t length) {
    smb360_apu_write(a,0x4010u,control);
    smb360_apu_write(a,0x4012u,addr);
    smb360_apu_write(a,0x4013u,length);
    smb360_apu_write(a,0x4015u,0x10u);
}
static void test_dmc_registers_and_irq(void) {
    smb360_apu a;
    smb360_apu_init(&a);
    assert(a.dmc.silence==1u && a.dmc.bits_remaining==8u);
    assert(smb360_apu_dmc_output(&a)==0u);
    smb360_apu_write(&a,0x4011u,0xffu);
    assert(smb360_apu_dmc_output(&a)==127u); /* high bit ignored */
    configure(&a,0x8fu,0xffu,0u);
    assert(a.dmc.rate_index==15u);
    assert(smb360_apu_dmc_dma_requested(&a));
    assert(smb360_apu_dmc_dma_address(&a)==0xffc0u);
    assert((smb360_apu_read_status(&a)&0x90u)==0x10u);
    smb360_apu_dmc_supply_byte(&a,0x5au);
    assert(a.dmc.buffer_full==1u && a.dmc.sample_buffer==0x5au);
    assert(a.dmc.fetched_bytes==1u && !a.dmc.bytes_remaining);
    assert(a.dmc.current_address==0xffc1u);
    assert(!smb360_apu_dmc_dma_requested(&a));
    assert(smb360_apu_irq_pending(&a));
    assert(smb360_apu_read_status(&a)==0x80u);
    assert(smb360_apu_read_status(&a)==0x80u); /* reading does not clear DMC IRQ */
    smb360_apu_write(&a,0x4010u,0x0fu);
    assert(smb360_apu_read_status(&a)==0u);
    assert(!smb360_apu_irq_pending(&a));
    smb360_apu_write(&a,0x4015u,0x10u); /* restart if bytes remaining zero */
    assert(smb360_apu_dmc_dma_address(&a)==0xffc0u);
    assert(a.dmc.bytes_remaining==1u);
    assert(!smb360_apu_dmc_dma_requested(&a)); /* buffer already full */
    smb360_apu_write(&a,0x4015u,0u);
    assert(a.dmc.bytes_remaining==0u && !a.dmc.dma_pending);
    assert(a.dmc.buffer_full==1u); /* buffered audio still plays */
}

static void test_wrap_loop_length(void) {
    smb360_apu a;
    smb360_apu_init(&a);
    configure(&a,0x4fu,0xffu,0xffu); /* loop with 4081 bytes */
    assert(a.dmc.bytes_remaining==4081u);
    assert(smb360_apu_dmc_dma_address(&a)==0xffc0u);
    a.dmc.current_address=0xffffu;
    smb360_apu_dmc_supply_byte(&a,0u);
    assert(a.dmc.current_address==0x8000u);
    assert(a.dmc.bytes_remaining==4080u);
    a.dmc.buffer_full=0u;
    a.dmc.bytes_remaining=1u;
    a.dmc.dma_pending=1u;
    smb360_apu_dmc_supply_byte(&a,0xffu);
    assert(a.dmc.bytes_remaining==4081u);
    assert(a.dmc.current_address==0xffc0u);
    assert(!a.dmc.irq_flag); /* loop inhibits DMC IRQ */
    assert(!a.dmc.dma_pending); /* full buffer prevents immediate new fetch */
    smb360_apu_write(&a,0x4010u,0x8fu); /* enable IRQ, disable loop */
    a.dmc.buffer_full=0u;
    a.dmc.dma_pending=1u;
    a.dmc.bytes_remaining=1u;
    smb360_apu_dmc_supply_byte(&a,0u);
    assert(a.dmc.irq_flag && !a.dmc.bytes_remaining);
    smb360_apu_write(&a,0x4015u,0u);
    assert(!a.dmc.irq_flag && !a.dmc.bytes_remaining);
}

static void test_timer_and_delta_saturation(void) {
    smb360_apu a;
    smb360_apu_init(&a);
    configure(&a,0x0fu,0x00u,0u);
    smb360_apu_write(&a,0x4011u,64u);
    smb360_apu_dmc_supply_byte(&a,0xa5u);
    smb360_apu_step(&a,1u+7u*54u); /* 8 output clocks at NTSC rate 15 */
    assert(a.dmc.bits_remaining==8u && !a.dmc.buffer_full);
    assert(a.dmc.silence==0u && a.dmc.shift_register==0xa5u);
    assert(smb360_apu_dmc_output(&a)==64u);
    smb360_apu_step(&a,54u); /* bit0=1 */
    assert(smb360_apu_dmc_output(&a)==66u);
    smb360_apu_step(&a,54u); /* bit1=0 */
    assert(smb360_apu_dmc_output(&a)==64u);
    smb360_apu_step(&a,54u); /* bit2=1 */
    assert(smb360_apu_dmc_output(&a)==66u);
    a.dmc.direct_load=127u;
    a.dmc.shift_register=0xffu;
    a.dmc.timer_divider=0u;
    smb360_apu_step(&a,1u);
    assert(smb360_apu_dmc_output(&a)==127u);
    a.dmc.direct_load=0u;
    a.dmc.shift_register=0u;
    a.dmc.timer_divider=0u;
    smb360_apu_step(&a,1u);
    assert(smb360_apu_dmc_output(&a)==0u);
}

static void test_rate_table(void) {
    static const uint16_t expected[16]={
        428,380,340,320,286,254,226,214,190,160,142,128,106,84,72,54
    };
    unsigned i;
    for(i=0u;i<16u;i++) {
        smb360_apu a;
        smb360_apu_init(&a);
        smb360_apu_write(&a,0x4010u,(uint8_t)i);
        smb360_apu_step(&a,1u);
        assert(a.dmc.timer_divider==expected[i]-1u);
        smb360_apu_step(&a,(uint32_t)expected[i]-1u);
        assert(a.dmc.timer_divider==0u);
        smb360_apu_step(&a,1u);
        assert(a.dmc.timer_divider==expected[i]-1u);
    }
}

static void test_bus_dma_and_cpu_irq(void) {
    static uint8_t prg[32768],chr[8192];
    static smb360_machine m;
    uint64_t before;
    prg[0x7ffcu]=0x00u;prg[0x7ffdu]=0x80u;
    prg[0x7ffeu]=0x00u;prg[0x7fffu]=0x90u;
    prg[0]=0xeau; /* NOP at RESET */
    prg[0x4000u]=0xa5u; /* only canonical-style PRG bus source $C000 */
    smb360_machine_init(&m,prg,chr);
    smb360_nrom_write(&m.bus,0x4010u,0x8fu);
    smb360_nrom_write(&m.bus,0x4012u,0u);
    smb360_nrom_write(&m.bus,0x4013u,0u);
    smb360_nrom_write(&m.bus,0x4015u,0x10u);
    assert(m.bus.apu.dmc.dma_pending==1u);
    before=m.cpu.cycles;
    assert(smb360_machine_step(&m)==1);
    assert(m.cpu.cycles-before==6u); /* 2 for NOP + modeled 4 DMA */
    assert(m.bus.apu.cpu_cycles==m.cpu.cycles);
    assert(m.bus.apu.dmc.sample_buffer==0xa5u);
    assert(m.bus.apu.dmc.fetched_bytes==1u);
    assert((smb360_nrom_read(&m.bus,0x4015u)&0x90u)==0x80u);
    assert(m.bus.apu.dmc.irq_flag==1u);
    m.cpu.p=(uint8_t)(m.cpu.p&~F_I);
    /* IRQ is polled on the next instruction boundary. */
    before=m.cpu.cycles;
    assert(smb360_machine_step(&m)==1);
    assert(m.cpu.cycles-before>=7u);
    assert(m.cpu.pc>=0x9000u);
    assert((m.cpu.p&F_I)!=0u);
    assert(m.bus.apu.cpu_cycles==m.cpu.cycles);
    smb360_nrom_write(&m.bus,0x4015u,0u);
    assert(m.bus.apu.dmc.irq_flag==0u);
}

int main(void) {
    test_dmc_registers_and_irq();
    test_wrap_loop_length();
    test_timer_and_delta_saturation();
    test_rate_table();
    test_bus_dma_and_cpu_irq();
    puts("APU DMC DMA/delta/timer/IRQ: PASS");
    return 0;
}
