#ifndef SMB360_CANONICAL_PPU_RENDER_H
#define SMB360_CANONICAL_PPU_RENDER_H
#include "nrom.h"
/* Functional per-dot compositor, without the hardware fetch pipeline. */
void smb360_ppu_render_line(smb360_nrom *m, unsigned y);
void smb360_ppu_render_pixel(smb360_nrom *m, unsigned x, unsigned y);
void smb360_ppu_increment_y(smb360_nrom *m);
#endif
