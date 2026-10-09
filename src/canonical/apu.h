#ifndef SMB360_CANONICAL_APU_H
#define SMB360_CANONICAL_APU_H
#include <stdint.h>

typedef struct {
    uint8_t control, sweep, length, duty_step;
    uint8_t envelope_start, envelope_divider, envelope_decay;
    uint8_t sweep_reload, sweep_divider;
    uint16_t period, timer_divider;
} smb360_apu_pulse;

/* Triangle: a CPU-clocked sequencer, separate 240 Hz linear/length gates.
 * Halting its sequencer holds the last 4-bit DAC level (not silence). */
typedef struct {
    uint8_t control, length, linear_counter, linear_reload_value;
    uint8_t linear_reload_flag, sequence_step;
    uint16_t period, timer_divider;
} smb360_apu_triangle;

typedef struct {
    smb360_apu_pulse pulse1, pulse2;
    smb360_apu_triangle triangle;
    uint64_t cpu_cycles;
    uint32_t frame_cycles;
    uint8_t enabled, frame_mode5, irq_inhibit, frame_irq;
    uint8_t frame_reset_delay;
    uint64_t quarter_clocks, half_clocks;
} smb360_apu;

void smb360_apu_init(smb360_apu *apu);
void smb360_apu_write(smb360_apu *apu, uint16_t address, uint8_t value);
uint8_t smb360_apu_read_status(smb360_apu *apu);
void smb360_apu_step(smb360_apu *apu, uint32_t cpu_cycles);
/* Individual pulse DAC levels (0..15), not mixed/resampled audio. */
uint8_t smb360_apu_pulse1_output(const smb360_apu *apu);
uint8_t smb360_apu_pulse2_output(const smb360_apu *apu);
/* Triangle's 0..15 DAC level is retained when length/linear counters halt. */
uint8_t smb360_apu_triangle_output(const smb360_apu *apu);

#endif
