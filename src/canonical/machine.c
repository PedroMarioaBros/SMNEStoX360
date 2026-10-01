#include "machine.h"
#include <string.h>

void smb360_machine_init(smb360_machine*m,const uint8_t*prg,const uint8_t*chr){
 memset(m,0,sizeof(*m));
 smb360_nrom_init(&m->bus,prg,chr);
 smb360_cpu6502_init(&m->cpu,&m->bus);
 smb360_ppu_timing_init(&m->ppu,&m->bus);
 smb360_cpu6502_reset(&m->cpu);
}
int smb360_machine_step(smb360_machine*m){
 if(!smb360_cpu6502_step(&m->cpu))return 0;
 smb360_ppu_timing_step(&m->ppu,m->cpu.last_cycles*3u);
 ++m->instructions;
 return 1;
}
uint64_t smb360_machine_run(smb360_machine*m,uint64_t n){
 uint64_t start=m->instructions;
 while(n-- && smb360_machine_step(m)){}
 return m->instructions-start;
}
