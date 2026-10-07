from pathlib import Path
p=Path('src/canonical/nrom.h');s=p.read_text().replace('uint8_t nmi_line, nmi_pending;','''uint8_t nmi_line, nmi_pending;
    uint8_t nametable[0x800], palette[32], oam[256];
    uint16_t ppu_v, ppu_t;
    uint8_t fine_x, ppu_read_buffer, ppu_open_bus, oam_addr;
    uint8_t dma_pending, dma_page;''')
s=s.replace('\n#endif','\nuint8_t smb360_nrom_ppu_read(const smb360_nrom *m, uint16_t addr);\nvoid smb360_nrom_ppu_write(smb360_nrom *m, uint16_t addr, uint8_t value);\n\n#endif');p.write_text(s)
p=Path('src/canonical/nrom.c');s=p.read_text();start=s.index('uint8_t smb360_nrom_read')
s=s[:start]+'''static unsigned palette_index(uint16_t a) {
    unsigned i=a&31u;
    if((i&0x13u)==0x10u) i&=15u;
    return i;
}
uint8_t smb360_nrom_ppu_read(const smb360_nrom *m,uint16_t a) {
    a&=0x3fffu;
    if(a<0x2000u) return m->chr[a];
    if(a<0x3f00u) return m->nametable[a&0x7ffu]; /* vertical A B A B */
    return m->palette[palette_index(a)];
}
void smb360_nrom_ppu_write(smb360_nrom *m,uint16_t a,uint8_t v) {
    a&=0x3fffu;
    if(a<0x2000u) return; /* CHR-ROM */
    if(a<0x3f00u) m->nametable[a&0x7ffu]=v;
    else m->palette[palette_index(a)]=v&0x3fu;
}
static void increment_v(smb360_nrom *m) {
    /* CPU access outside rendering; rendering-time increment quirks pending. */
    m->ppu_v=(uint16_t)((m->ppu_v+((m->ppu_regs[0]&4u)?32u:1u))&0x7fffu);
}
uint8_t smb360_nrom_read(smb360_nrom *m, uint16_t a) {
    if (a < 0x2000u) return m->ram[a & 0x07ffu];
    if (a < 0x4000u) {
        uint8_t r=(uint8_t)(a&7u),v=m->ppu_open_bus;
        if(r==2u) {
            v=(uint8_t)((m->ppu_status&0xe0u)|(v&0x1fu));
            m->ppu_status&=(uint8_t)~0x80u;
            m->ppu_write_latch=0;update_nmi(m);
        } else if(r==4u) v=m->oam[m->oam_addr];
        else if(r==7u) {
            uint16_t addr=m->ppu_v&0x3fffu;
            uint8_t data=smb360_nrom_ppu_read(m,addr);
            if(addr>=0x3f00u) {
                v=(uint8_t)((v&0xc0u)|(data&((m->ppu_regs[1]&1u)?0x30u:0x3fu)));
                m->ppu_read_buffer=smb360_nrom_ppu_read(m,(uint16_t)(addr-0x1000u));
            } else {v=m->ppu_read_buffer;m->ppu_read_buffer=data;}
            increment_v(m);
        }
        m->ppu_open_bus=v;return v;
    }
    if(a==0x4016u) {
        uint8_t v=(uint8_t)((m->controller_strobe?m->controller1:m->controller_shift)&1u);
        if(!m->controller_strobe)
            m->controller_shift=(uint8_t)((m->controller_shift>>1)|0x80u);
        return v;
    }
    if(a>=0x4000u && a<=0x4017u) return m->apu_io[a-0x4000u];
    if(a>=0x8000u) return m->prg[a-0x8000u];
    return 0; /* CPU open bus and expansion hardware are not yet modeled. */
}
void smb360_nrom_write(smb360_nrom *m,uint16_t a,uint8_t v) {
    if(a<0x2000u){m->ram[a&0x7ffu]=v;return;}
    if(a<0x4000u){
        uint8_t r=(uint8_t)(a&7u);
        m->ppu_open_bus=v;
        if(r!=2u) m->ppu_regs[r]=v;
        switch(r){
        case 0:
            m->ppu_t=(uint16_t)((m->ppu_t&0x73ffu)|((v&3u)<<10));
            update_nmi(m);break;
        case 3:m->oam_addr=v;break;
        case 4:m->oam[m->oam_addr++]=v;break;
        case 5:
            if(!m->ppu_write_latch){
                m->fine_x=v&7u;m->ppu_t=(uint16_t)((m->ppu_t&0x7fe0u)|(v>>3));
            }else m->ppu_t=(uint16_t)((m->ppu_t&0x0c1fu)|((v&7u)<<12)|((v&0xf8u)<<2));
            m->ppu_write_latch^=1u;break;
        case 6:
            if(!m->ppu_write_latch)m->ppu_t=(uint16_t)((m->ppu_t&0xffu)|((v&0x3fu)<<8));
            else {m->ppu_t=(uint16_t)((m->ppu_t&0x7f00u)|v);m->ppu_v=m->ppu_t;}
            m->ppu_write_latch^=1u;break;
        case 7:smb360_nrom_ppu_write(m,m->ppu_v,v);increment_v(m);break;
        default:break;
        }
        return;
    }
    if(a==0x4014u){m->dma_page=v;m->dma_pending=1;return;}
    if(a==0x4016u){
        uint8_t strobe=v&1u;
        if(strobe || m->controller_strobe)m->controller_shift=m->controller1;
        m->controller_strobe=strobe;return;
    }
    if(a>=0x4000u && a<=0x4017u)m->apu_io[a-0x4000u]=v;
}
'''
p.write_text(s)
p=Path('src/canonical/machine.c');s=p.read_text().replace(' ++m->instructions;', ''' if(m->bus.dma_pending){
  unsigned i;uint32_t stall=513u+(uint32_t)(m->cpu.cycles&1u);
  m->bus.dma_pending=0;
  for(i=0;i<256;i++)m->bus.oam[m->bus.oam_addr++]=
   smb360_nrom_read(&m->bus,(uint16_t)(((uint16_t)m->bus.dma_page<<8)|i));
  m->cpu.cycles+=stall;
  smb360_ppu_timing_step(&m->ppu,stall*3u);
 }
 ++m->instructions;''');p.write_text(s)
