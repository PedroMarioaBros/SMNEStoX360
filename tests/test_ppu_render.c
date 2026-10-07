#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../src/canonical/ppu_render.h"
#include "../src/canonical/ppu_timing.h"
static smb360_nrom m;
int main(void){
 uint8_t prg[32768]={0},chr[8192]={0};smb360_ppu_timing p;
 smb360_nrom_init(&m,prg,chr);
 memset(m.oam,0xff,sizeof(m.oam));m.oam[0]=0;m.oam[1]=1;m.oam[2]=0;m.oam[3]=8;
 chr[0]=0x80;chr[16]=0x80;
 m.palette[0]=1;m.palette[1]=2;m.palette[17]=3;m.ppu_regs[1]=0x1e;
 smb360_ppu_render_line(&m,1);smb360_ppu_render_pixel(&m,8,1);
 assert((m.ppu_status&0x40)&&m.sprite_zero_hits==1&&m.pixels[264]==3);
 m.oam[2]=0x20;smb360_ppu_render_line(&m,1);smb360_ppu_render_pixel(&m,8,1);
 assert(m.pixels[264]==2);
 m.ppu_status=0;chr[0]=0;smb360_ppu_render_pixel(&m,8,1);assert(!(m.ppu_status&0x40));
 chr[0]=1;m.oam[3]=255;smb360_ppu_render_line(&m,1);smb360_ppu_render_pixel(&m,255,1);
 assert(!(m.ppu_status&0x40));
 chr[0]=0x80;m.oam[3]=0;m.ppu_regs[1]=0x18;
 smb360_ppu_render_line(&m,1);smb360_ppu_render_pixel(&m,0,1);assert(!(m.ppu_status&0x40));
 m.ppu_v=(uint16_t)(0x7000|(29<<5));smb360_ppu_increment_y(&m);assert(m.ppu_v==0x0800);
 m.ppu_v=(uint16_t)(0x7000|(31<<5));smb360_ppu_increment_y(&m);assert(m.ppu_v==0);
 smb360_ppu_timing_init(&p,&m);p.scanline=261;m.ppu_status=0xe0;
 smb360_ppu_timing_step(&p,1);assert(!(m.ppu_status&0xe0));
 puts("PPU pixels/sprite-zero/priority/clipping/Y increment: PASS");
 return 0;
}
