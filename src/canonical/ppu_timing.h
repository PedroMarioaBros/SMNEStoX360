#ifndef SMB360_CANONICAL_PPU_TIMING_H
#define SMB360_CANONICAL_PPU_TIMING_H
#include <stdint.h>
#include "nrom.h"

typedef struct {
    uint32_t dot;
    uint16_t scanline;
    uint64_t frame;
    smb360_nrom *bus;
} smb360_ppu_timing;

void smb360_ppu_timing_init(smb360_ppu_timing *p, smb360_nrom *bus);
void smb360_ppu_timing_step(smb360_ppu_timing *p, uint32_t ppu_cycles);

#endif
