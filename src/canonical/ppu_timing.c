#include "ppu_timing.h"
#include "ppu_render.h"
#include <string.h>

void smb360_ppu_timing_init(smb360_ppu_timing *p, smb360_nrom *bus) {
    memset(p,0,sizeof(*p));
    p->bus=bus;
}

void smb360_ppu_timing_step(smb360_ppu_timing *p, uint32_t n) {
    while(n--) {
        /* NTSC: with rendering enabled, odd frames omit pre-render dot 340.
         * The frame counter starts even and advances even when rendering is off.
         * See https://www.nesdev.org/wiki/PPU_frame_timing . */
        if (p->scanline==261u && p->dot==339u && (p->frame&1u) &&
            (p->bus->ppu_regs[1]&0x18u)) {
            p->dot=0u;
            p->scanline=0u;
            ++p->frame;
            continue;
        }
        ++p->dot;
        if (p->scanline<240u && p->dot>=1u && p->dot<=256u) {
            if(p->dot==1u)smb360_ppu_render_line(p->bus,p->scanline);
            smb360_ppu_render_pixel(p->bus,p->dot-1u,p->scanline);
        }
        if((p->bus->ppu_regs[1]&0x18u) && (p->scanline<240u || p->scanline==261u)){
            if(p->dot==256u)smb360_ppu_increment_y(p->bus);
            if(p->dot==257u)p->bus->ppu_v=(uint16_t)((p->bus->ppu_v&0x7be0u)|(p->bus->ppu_t&0x041fu));
            if(p->scanline==261u && p->dot>=280u && p->dot<=304u)
                p->bus->ppu_v=(uint16_t)((p->bus->ppu_v&0x041fu)|(p->bus->ppu_t&0x7be0u));
        }
        if (p->scanline==241u && p->dot==1u){
            memcpy(p->bus->completed_pixels,p->bus->pixels,sizeof(p->bus->pixels));
            smb360_nrom_set_vblank(p->bus,1);
        }
        if (p->scanline==261u && p->dot==1u){
            p->bus->ppu_status&=0x1fu;
            smb360_nrom_set_vblank(p->bus,0);
        }
        if (p->dot>=341u) {
            p->dot=0u;
            ++p->scanline;
            if (p->scanline>=262u) {
                p->scanline=0u;
                ++p->frame;
            }
        }
    }
}
