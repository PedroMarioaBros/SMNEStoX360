#include "machine.h"
#include <string.h>

void smb360_machine_init(smb360_machine*m,const uint8_t*prg,const uint8_t*chr){
 memset(m,0,sizeof(*m));
 smb360_nrom_init(&m->bus,prg,chr);
 smb360_cpu6502_init(&m->cpu,&m->bus);
 smb360_ppu_timing_init(&m->ppu,&m->bus);
 smb360_cpu6502_reset(&m->cpu);
 smb360_ppu_timing_step(&m->ppu,m->cpu.last_cycles*3u);
 smb360_apu_step(&m->bus.apu,m->cpu.last_cycles);
}
int smb360_machine_step(smb360_machine*m){
 if(m->cpu.stopped)return 0;
 if(m->bus.nmi_pending){
  m->bus.nmi_pending=0;
  smb360_cpu6502_nmi(&m->cpu);
  if(m->nmi_count++==0){
   m->first_nmi_cycle=m->cpu.cycles;
   m->first_nmi_frame=m->ppu.frame;
   m->first_nmi_pc=m->cpu.pc;
  }
  smb360_ppu_timing_step(&m->ppu,m->cpu.last_cycles*3u);
  smb360_apu_step(&m->bus.apu,m->cpu.last_cycles);
 }
 if(!smb360_cpu6502_step(&m->cpu))return 0;
 smb360_ppu_timing_step(&m->ppu,m->cpu.last_cycles*3u);
 smb360_apu_step(&m->bus.apu,m->cpu.last_cycles);
 if(m->bus.dma_pending){
  unsigned i;uint32_t stall=513u+(uint32_t)(m->cpu.cycles&1u);
  m->bus.dma_pending=0;
  for(i=0;i<256;i++)m->bus.oam[m->bus.oam_addr++]=
   smb360_nrom_read(&m->bus,(uint16_t)(((uint16_t)m->bus.dma_page<<8)|i));
  m->cpu.cycles+=stall;
  smb360_ppu_timing_step(&m->ppu,stall*3u);
  smb360_apu_step(&m->bus.apu,stall);
 }
 ++m->instructions;
 return 1;
}
uint64_t smb360_machine_run(smb360_machine*m,uint64_t n){
 uint64_t start=m->instructions;
 while(n-- && smb360_machine_step(m)){}
 return m->instructions-start;
}
