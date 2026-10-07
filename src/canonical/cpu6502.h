#ifndef SMB360_CANONICAL_CPU6502_H
#define SMB360_CANONICAL_CPU6502_H
#include <stdint.h>
#include "nrom.h"

enum { F_C=1, F_Z=2, F_I=4, F_D=8, F_B=16, F_U=32, F_V=64, F_N=128 };

typedef struct {
    uint8_t a,x,y,s,p;
    uint16_t pc;
    uint64_t cycles;
    int stopped;
    uint32_t last_cycles;
    uint64_t opcode_hits[256];
    smb360_nrom *bus;
} smb360_cpu6502;

void smb360_cpu6502_init(smb360_cpu6502 *c, smb360_nrom *bus);
void smb360_cpu6502_reset(smb360_cpu6502 *c);
int smb360_cpu6502_step(smb360_cpu6502 *c);

#endif
