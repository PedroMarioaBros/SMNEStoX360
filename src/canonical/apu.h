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

/* NTSC 2A03 noise: CPU/2-clocked 15-bit LFSR plus envelope/length. */
typedef struct {
    uint8_t control, mode, period_index, length;
    uint8_t envelope_start, envelope_divider, envelope_decay;
    uint16_t timer_divider, shift_register;
} smb360_apu_noise;

/* DMC: CPU-clocked output unit with a one-byte DMA sample buffer.
 * The CPU/machine must service a pending request using the canonical bus. */
typedef struct {
    uint8_t control, rate_index, direct_load;
    uint8_t sample_address, sample_length;
    uint8_t shift_register, bits_remaining, silence;
    uint8_t sample_buffer, buffer_full, irq_flag, dma_pending;
    uint16_t timer_divider, current_address, bytes_remaining;
    uint64_t fetched_bytes;
} smb360_apu_dmc;

typedef struct {
    smb360_apu_pulse pulse1, pulse2;
    smb360_apu_triangle triangle;
    smb360_apu_noise noise;
    smb360_apu_dmc dmc;
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
/* Noise DAC level 0..15; bit 0 of the LFSR and length gate the output. */
uint8_t smb360_apu_noise_output(const smb360_apu *apu);
/* DMC 7-bit DAC level, without mixer or audible PCM output. */
uint8_t smb360_apu_dmc_output(const smb360_apu *apu);
/* The machine performs a DMA read at this address and supplies the byte. */
int smb360_apu_dmc_dma_requested(const smb360_apu *apu);
uint16_t smb360_apu_dmc_dma_address(const smb360_apu *apu);
void smb360_apu_dmc_supply_byte(smb360_apu *apu, uint8_t value);
/* APU interrupt lines, intended for CPU instruction-boundary polling. */
int smb360_apu_irq_pending(const smb360_apu *apu);

#endif
