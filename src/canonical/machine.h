#ifndef SMB360_CANONICAL_MACHINE_H
#define SMB360_CANONICAL_MACHINE_H
#include <stdint.h>
#include "nrom.h"
#include "cpu6502.h"
#include "ppu_timing.h"

typedef struct {
 smb360_nrom bus;
 smb360_cpu6502 cpu;
 smb360_ppu_timing ppu;
 uint64_t instructions;
} smb360_machine;

void smb360_machine_init(smb360_machine *m,const uint8_t *prg,const uint8_t *chr);
int smb360_machine_step(smb360_machine *m);
uint64_t smb360_machine_run(smb360_machine *m,uint64_t max_instructions);

#endif
