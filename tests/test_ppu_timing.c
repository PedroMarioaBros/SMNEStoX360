#include <assert.h>
#include <stdint.h>
#include <string.h>
#include "../src/canonical/nrom.h"
#include "../src/canonical/ppu_timing.h"

static const uint8_t empty_prg[SMB360_PRG_SIZE]={0};
static const uint8_t empty_chr[SMB360_CHR_SIZE]={0};

static void frame_lengths(uint8_t mask) {
 smb360_nrom m;
 smb360_ppu_timing p;
 unsigned frame;
 smb360_nrom_init(&m,empty_prg,empty_chr);
 smb360_ppu_timing_init(&p,&m);
 m.ppu_regs[1]=mask;
 for(frame=0;frame<6;frame++) {
  unsigned length=89342u-((frame&1u) && (mask&0x18u)?1u:0u);
  smb360_ppu_timing_step(&p,length-1u);
  assert(p.frame==frame && p.scanline==261u);
  assert(p.dot==((frame&1u) && (mask&0x18u)?339u:340u));
  smb360_ppu_timing_step(&p,1);
  assert(p.frame==frame+1u && p.scanline==0 && p.dot==0);
 }
}

static void rendering_toggle(void) {
 smb360_nrom m;
 smb360_ppu_timing p;
 smb360_nrom_init(&m,empty_prg,empty_chr);
 smb360_ppu_timing_init(&p,&m);
 /* A disabled frame still toggles parity. Enable just before the skip. */
 smb360_ppu_timing_step(&p,89342u+261u*341u+339u);
 assert(p.frame==1 && p.scanline==261 && p.dot==339);
 m.ppu_regs[1]=0x08;
 smb360_ppu_timing_step(&p,1);
 assert(p.frame==2 && p.scanline==0 && p.dot==0);
 /* Conversely, disabling at the boundary keeps both remaining clocks. */
 smb360_ppu_timing_step(&p,89342u+261u*341u+339u);
 m.ppu_regs[1]=0;
 smb360_ppu_timing_step(&p,1);
 assert(p.frame==3 && p.scanline==261 && p.dot==340);
 smb360_ppu_timing_step(&p,1);
 assert(p.frame==4 && p.scanline==0 && p.dot==0);
 /* Enabling after dot 339 must not retroactively skip a clock. */
 smb360_ppu_timing_step(&p,89342u+261u*341u+340u);
 m.ppu_regs[1]=0x18;
 smb360_ppu_timing_step(&p,1);
 assert(p.frame==6 && p.scanline==0 && p.dot==0);
}

int main(void) {
 uint8_t prg[SMB360_PRG_SIZE]={0}, chr[SMB360_CHR_SIZE]={0};
 smb360_nrom m; smb360_ppu_timing p;
 smb360_nrom_init(&m,prg,chr); smb360_ppu_timing_init(&p,&m);
 assert((smb360_nrom_read(&m,0x2002)&0x80u)==0);
 smb360_ppu_timing_step(&p,241u*341u+1u);
 assert((m.ppu_status&0x80u)!=0);
 assert((smb360_nrom_read(&m,0x2002)&0x80u)!=0);
 assert((m.ppu_status&0x80u)==0);
 m.ppu_write_latch=1;
 smb360_nrom_set_vblank(&m,1);
 (void)smb360_nrom_read(&m,0x2002);
 assert(m.ppu_write_latch==0);
 frame_lengths(0);
 frame_lengths(0x08);
 frame_lengths(0x10);
 frame_lengths(0x18);
 rendering_toggle();
 return 0;
}
