#include "ppu_timing.h"
#include <string.h>

void smb360_ppu_timing_init(smb360_ppu_timing *p, smb360_nrom *bus) {
    memset(p,0,sizeof(*p));
    p->bus=bus;
}

void smb360_ppu_timing_step(smb360_ppu_timing *p, uint32_t n) {
    while(n--) {
        ++p->dot;
        if (p->scanline==241u && p->dot==1u)
            smb360_nrom_set_vblank(p->bus,1);
        if (p->scanline==261u && p->dot==1u)
            smb360_nrom_set_vblank(p->bus,0);
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
