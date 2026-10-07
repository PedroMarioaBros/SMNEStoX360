#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../src/canonical/machine.h"
static uint8_t prg[32768],chr[8192];
static smb360_machine m;
static void address(uint16_t a) {
 (void)smb360_nrom_read(&m.bus,0x2002);
 smb360_nrom_write(&m.bus,0x2006,(uint8_t)(a>>8));
 smb360_nrom_write(&m.bus,0x2006,(uint8_t)a);
}
int main(void){
 unsigned i; uint64_t before; uint8_t original;
 prg[0x7ffc]=0;prg[0x7ffd]=0x80;
 smb360_machine_init(&m,prg,chr);
 smb360_nrom_ppu_write(&m.bus,0x2000,0x42);
 assert(smb360_nrom_ppu_read(&m.bus,0x2800)==0x42);
 assert(smb360_nrom_ppu_read(&m.bus,0x3000)==0x42);
 assert(smb360_nrom_ppu_read(&m.bus,0x2400)==0);
 smb360_nrom_ppu_write(&m.bus,0x3f10,0xff);
 assert(smb360_nrom_ppu_read(&m.bus,0x3f00)==0x3f);
 assert(smb360_nrom_ppu_read(&m.bus,0x3f30)==0x3f);
 chr[0]=0x81;smb360_nrom_ppu_write(&m.bus,0,0);
 assert(smb360_nrom_ppu_read(&m.bus,0)==0x81);
 address(0x2000);assert(smb360_nrom_read(&m.bus,0x2007)==0);
 assert(smb360_nrom_read(&m.bus,0x2007)==0x42);
 smb360_nrom_ppu_write(&m.bus,0x2f00,0x67);address(0x3f00);
 assert((smb360_nrom_read(&m.bus,0x2007)&63)==63);
 assert(m.bus.ppu_read_buffer==0x67);
 smb360_nrom_write(&m.bus,0x2000,4);address(0x2400);
 smb360_nrom_write(&m.bus,0x2007,0x11);smb360_nrom_write(&m.bus,0x2007,0x22);
 assert(smb360_nrom_ppu_read(&m.bus,0x2420)==0x22&&m.bus.ppu_v==0x2440);
 smb360_nrom_write(&m.bus,0x2005,0x2b);
 assert(m.bus.fine_x==3&&(m.bus.ppu_t&31)==5);
 smb360_nrom_write(&m.bus,0x2005,0x55);
 assert((m.bus.ppu_t&0x73e0)==0x5140);
 smb360_nrom_write(&m.bus,0x2005,1);
 smb360_nrom_set_vblank(&m.bus,1);
 assert(smb360_nrom_read(&m.bus,0x2002)==0x81);
 assert(!m.bus.ppu_write_latch&&!m.bus.nmi_line);
 smb360_nrom_write(&m.bus,0x2003,0xff);smb360_nrom_write(&m.bus,0x2004,0x56);
 assert(m.bus.oam_addr==0&&m.bus.oam[255]==0x56);
 smb360_nrom_set_controller1(&m.bus,0);smb360_nrom_write(&m.bus,0x4016,1);
 smb360_nrom_set_controller1(&m.bus,0xa5);assert(smb360_nrom_read(&m.bus,0x4016)==1);
 smb360_nrom_write(&m.bus,0x4016,0);
 for(i=0;i<8;i++)assert(smb360_nrom_read(&m.bus,0x4016)==((0xa5u>>i)&1));
 assert(smb360_nrom_read(&m.bus,0x4016)==1);
 for(i=0;i<256;i++)m.bus.ram[0x200+i]=(uint8_t)i;
 prg[0]=0x8d;prg[1]=0x14;prg[2]=0x40;m.cpu.a=2;
 m.bus.oam_addr=0xf0;before=m.cpu.cycles;
 assert(smb360_machine_step(&m));assert(m.cpu.cycles-before==518);
 for(i=0;i<256;i++)assert(m.bus.oam[(0xf0+i)&255]==i);
 assert(m.bus.oam_addr==0xf0);
 original=m.bus.oam[0];
 smb360_nrom_write(&m.bus,0x2002,0xff);assert(m.bus.oam[0]==original);
 /* Opposite CPU parity: 4 cycles STA + 513 DMA. */
 m.cpu.pc=0x8000;m.cpu.cycles=8;before=m.cpu.cycles;
 assert(smb360_machine_step(&m));assert(m.cpu.cycles-before==517);
 puts("PPU registers/memory/controller/OAM DMA: PASS");return 0;
}
