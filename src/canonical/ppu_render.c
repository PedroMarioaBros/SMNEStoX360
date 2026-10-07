#include "ppu_render.h"
void smb360_ppu_render_line(smb360_nrom *m,unsigned y) {
 unsigned i,height=(m->ppu_regs[0]&0x20u)?16u:8u;
 m->line_v=m->ppu_v;m->line_sprites=0;
 for(i=0;i<64;i++){
  unsigned top=(unsigned)m->oam[4*i]+1u;
  if(y<top || y>=top+height)continue;
  if(m->line_sprites==8){
   if(m->ppu_regs[1]&0x18u)m->ppu_status|=0x20u;
   break; /* Simple overflow; hardware evaluation bug remains unmodeled. */
  }
  m->sprite_indices[m->line_sprites++]=(uint8_t)i;
 }
}
static uint8_t pattern(const smb360_nrom *m,unsigned addr,unsigned bit) {
 return (uint8_t)(((smb360_nrom_ppu_read(m,(uint16_t)addr)>>bit)&1u)|
  (((smb360_nrom_ppu_read(m,(uint16_t)(addr+8))>>bit)&1u)<<1));
}
void smb360_ppu_render_pixel(smb360_nrom *m,unsigned x,unsigned y) {
 unsigned bg=0,bgpal=0,sp=0,sppal=0,behind=0,i;
 unsigned mask=m->ppu_regs[1];
 if((mask&8u)&&(x>=8||(mask&2u))){
  unsigned sx=(m->line_v&31u)*8u+m->fine_x+x;
  unsigned tx=(sx>>3)&31u,ty=(m->line_v>>5)&31u;
  unsigned nt=(m->line_v&0xc00u)^((sx&256u)?0x400u:0);
  unsigned base=0x2000u|nt;
  unsigned tile=smb360_nrom_ppu_read(m,(uint16_t)(base+ty*32u+tx));
  unsigned attr=smb360_nrom_ppu_read(m,(uint16_t)(base+0x3c0u+(ty>>2)*8u+(tx>>2)));
  unsigned bank=(m->ppu_regs[0]&0x10u)?0x1000u:0;
  bg=pattern(m,bank+tile*16u+((m->line_v>>12)&7u),7u-(sx&7u));
  bgpal=(attr>>(((ty&2u)<<1)|(tx&2u)))&3u;
 }
 if((mask&16u)&&(x>=8||(mask&4u))){
  unsigned height=(m->ppu_regs[0]&0x20u)?16u:8u;
  for(i=0;i<m->line_sprites;i++){
   unsigned index=m->sprite_indices[i],o=index*4u,left=m->oam[o+3];
   unsigned row,col,attr,tile,bank,v;
   if(x<left||x>=left+8u)continue;
   row=y-(unsigned)m->oam[o]-1u;col=x-left;attr=m->oam[o+2];tile=m->oam[o+1];
   if(attr&0x80u)row=height-1-row;
   if(attr&0x40u)col=7-col;
   if(height==16){bank=(tile&1u)*0x1000u;tile=(tile&0xfeu)+(row>>3);row&=7;}
   else bank=(m->ppu_regs[0]&8u)?0x1000u:0;
   v=pattern(m,bank+tile*16u+row,7-col);
   if(!v)continue;
   if(index==0&&bg&&x!=255&&!(m->ppu_status&0x40u)){
    m->ppu_status|=0x40u;++m->sprite_zero_hits;
   }
   sp=v;sppal=attr&3u;behind=attr&0x20u;break;
  }
 }
 {
  unsigned pal=bg ? bgpal*4u+bg : 0;
  uint8_t color;
  if(sp&&(!bg||!behind))pal=16u+sppal*4u+sp;
  color=smb360_nrom_ppu_read(m,(uint16_t)(0x3f00u+pal));
  if(mask&1u)color&=0x30u;
  m->pixels[y*256u+x]=color; /* 6-bit palette indices, not composite RGB. */
 }
}
void smb360_ppu_increment_y(smb360_nrom *m) {
 unsigned y;
 if((m->ppu_v&0x7000u)!=0x7000u){m->ppu_v+=0x1000u;return;}
 m->ppu_v&=0x0fffu;y=(m->ppu_v&0x03e0u)>>5;
 if(y==29){y=0;m->ppu_v^=0x0800u;}
 else if(y==31)y=0;
 else ++y;
 m->ppu_v=(uint16_t)((m->ppu_v&~0x03e0u)|(y<<5));
}
