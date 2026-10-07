#include <stdio.h>
#include <stdint.h>
#include "src/canonical/machine.h"
static smb360_machine m;
int main(void){
 uint8_t rom[40976];FILE*f=fopen("../rom/SMB_v026.nes","rb");if(!f)return 1;
 if(fread(rom,1,sizeof rom,f)!=sizeof rom)return 2;fclose(f);
 smb360_machine_init(&m,rom+16,rom+32784);
 for(unsigned i=0;i<5000000;i++){
  unsigned fr=(unsigned)m.ppu.frame;
  smb360_nrom_set_controller1(&m.bus,fr>=100&&fr<102?8:fr>=220&&fr<270?129:fr>=270&&fr<360?128:0);
  if(fr==500&&m.ppu.scanline<40&&m.ppu.dot<12)
   printf("line=%u dot=%u pc=%04x v=%04x t=%04x mask=%02x status=%02x fine=%u\n",
    m.ppu.scanline,m.ppu.dot,m.cpu.pc,m.bus.ppu_v,m.bus.ppu_t,m.bus.ppu_regs[1],m.bus.ppu_status,m.bus.fine_x);
  if(!smb360_machine_step(&m))break;
 }
 return 0;
}
