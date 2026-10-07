from pathlib import Path
p=Path('src/canonical/nrom.h');s=p.read_text().replace('uint8_t ppu_write_latch;','uint8_t ppu_write_latch;\n    uint8_t nmi_line, nmi_pending;');p.write_text(s)
p=Path('src/canonical/nrom.c');s=p.read_text();s=s.replace('void smb360_nrom_set_vblank', '''static void update_nmi(smb360_nrom *m) {
    uint8_t line=(uint8_t)((m->ppu_status & m->ppu_regs[0] & 0x80u)!=0);
    if(line && !m->nmi_line) m->nmi_pending=1;
    m->nmi_line=line;
}

void smb360_nrom_set_vblank''')
s=s.replace('else m->ppu_status &= (uint8_t)~0x80u;','else m->ppu_status &= (uint8_t)~0x80u;\n    update_nmi(m);')
s=s.replace('return v;\n        }','update_nmi(m);\n            return v;\n        }')
s=s.replace('m->ppu_regs[r] = v;','m->ppu_regs[r] = v;\n        if (r == 0u) update_nmi(m);')
p.write_text(s)
p=Path('src/canonical/machine.h');s=p.read_text().replace('uint64_t instructions;','uint64_t instructions;\n uint64_t nmi_count, first_nmi_cycle, first_nmi_frame;\n uint16_t first_nmi_pc;');p.write_text(s)
p=Path('src/canonical/machine.c');s=p.read_text().replace('smb360_cpu6502_reset(&m->cpu);','smb360_cpu6502_reset(&m->cpu);\n smb360_ppu_timing_step(&m->ppu,m->cpu.last_cycles*3u);')
s=s.replace(' if(!smb360_cpu6502_step', ''' if(m->cpu.stopped)return 0;
 if(m->bus.nmi_pending){
  m->bus.nmi_pending=0;
  smb360_cpu6502_nmi(&m->cpu);
  if(m->nmi_count++==0){
   m->first_nmi_cycle=m->cpu.cycles;
   m->first_nmi_frame=m->ppu.frame;
   m->first_nmi_pc=m->cpu.pc;
  }
  smb360_ppu_timing_step(&m->ppu,m->cpu.last_cycles*3u);
 }
 if(!smb360_cpu6502_step''');p.write_text(s)
p=Path('tests/run_canonical_rom.c');s=p.read_text().replace(' free(rom);\n return 0;', ''' printf("nmi_count=%llu first_nmi_pc=$%04X first_nmi_cycle=%llu first_nmi_frame=%llu last_opcode_pc=$%04X\\n",
 (unsigned long long)m.nmi_count,m.first_nmi_pc,(unsigned long long)m.first_nmi_cycle,
 (unsigned long long)m.first_nmi_frame,m.cpu.opcode_pc);
 free(rom);
 return m.cpu.stopped ? 1 : 0;''');p.write_text(s)
