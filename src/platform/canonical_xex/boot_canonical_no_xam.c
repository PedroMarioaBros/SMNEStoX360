/* S016 boot isolation, stage 02: exact owner's 6502 PRG, no XAM.
 * Only run on real Xbox if stage 01 has safely returned to dashboard.
 * NO graphics/MMIO, NO XAM notifications/input, NO stdio/printf.
 * The canonical machine runs a bounded CPU/PPU test, then exits.
 * Failure in this stage after 01 works narrows the cause to game
 * runtime or binary image differences, not proof of either. */
#include <stddef.h>
#include <stdint.h>
#include <xecore/xboxkrnl.h>
#include "../../canonical/machine.h"

#define ROM_BYTES 40976u
#define FRAME_COUNT 120u
#define FRAME_LIMIT 1000000u
extern const uint8_t smb360_canonical_rom[];
extern const uint8_t smb360_canonical_rom_end[];
static smb360_machine m;

static int rom_ok(void) {
    const uint8_t *r=smb360_canonical_rom;
    return (size_t)(smb360_canonical_rom_end-r)==ROM_BYTES &&
           r[0]==0x4eu && r[1]==0x45u && r[2]==0x53u &&
           r[3]==0x1au && r[4]==2u && r[5]==1u;
}

int main(void) {
    uint32_t frame;
    if (!rom_ok()) return 3;
    smb360_machine_init(&m,smb360_canonical_rom+16u,
                        smb360_canonical_rom+16u+SMB360_PRG_SIZE);
    for(frame=0u;frame<FRAME_COUNT;frame++) {
        uint32_t n=0u;
        for(;;) {
            if(n++>=FRAME_LIMIT) return 5;
            if(!smb360_machine_step(&m)) return 4;
            if(m.ppu.frame>=frame && m.ppu.scanline>=241u &&
               m.ppu.dot>=1u) break;
        }
    }
    /* Return to dashboard only after emulator CPU and PPU completed. */
    {
        int64_t five_seconds=-50000000ll;
        KeDelayExecutionThread(0u,0u,&five_seconds);
    }
    return 0;
}
