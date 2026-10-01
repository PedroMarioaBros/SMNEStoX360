#include <assert.h>
#include <stdint.h>
#include <string.h>
#include "../src/canonical/nrom.h"
#include "../src/canonical/ppu_timing.h"

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
 return 0;
}
