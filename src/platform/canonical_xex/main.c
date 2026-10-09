/* S014: CANONICAL ROM diagnostic only. No unsafe framebuffer addresses.
 * Executes PRG/CHR from SMB_v026.nes through src/canonical/*.c.
 * Not a playable Xbox port: output currently goes to diagnostic stdout.
 * CPU6502 is still interpreted in C; no native PPC translation is claimed. */
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <xecore/xboxkrnl.h>
#include "../../canonical/machine.h"

#define NES_ROM_BYTES 40976u
#define PROBE_FRAMES 120u
#define MAX_STEPS_PER_FRAME 1000000u

extern const uint8_t smb360_canonical_rom[];
extern const uint8_t smb360_canonical_rom_end[];

static smb360_machine machine;

static uint32_t fnv1a(const uint8_t *data, size_t size) {
    uint32_t value=2166136261u;
    size_t i;
    for (i=0;i<size;i++) {
        value^=data[i];
        value*=16777619u;
    }
    return value;
}

static int check_embedded_rom(void) {
    size_t size=(size_t)(smb360_canonical_rom_end-smb360_canonical_rom);
    const uint8_t *p=smb360_canonical_rom;
    /* Complete canonical iNES file, not just artwork or substituted PRG. */
    if(size!=NES_ROM_BYTES) return 0;
    if(p[0]!='N' || p[1]!='E' || p[2]!='S' || p[3]!=0x1au ||
       p[4]!=2u || p[5]!=1u || (p[6]&0xf0u)!=0u) return 0;
    return 1;
}

int main(void) {
    unsigned frame;
    uint32_t first_hash=0u;
    printf("SMB360 CANONICAL XEX S014 diagnostic: startup\n");
    if(!check_embedded_rom()) {
        printf("SMB360 CANONICAL XEX: embedded iNES header/size INVALID\n");
        return 3;
    }
    printf("SMB360 CANONICAL XEX: ROM bytes=%u, PRG=%u CHR=%u\n",
           (unsigned)NES_ROM_BYTES,(unsigned)SMB360_PRG_SIZE,
           (unsigned)SMB360_CHR_SIZE);
    smb360_machine_init(&machine,smb360_canonical_rom+16u,
                        smb360_canonical_rom+16u+SMB360_PRG_SIZE);
    for(frame=0u;frame<PROBE_FRAMES;frame++) {
        unsigned steps=0u;
        /* Same VBlank capture policy as the host bench. */
        smb360_nrom_set_controller1(&machine.bus,0u);
        smb360_nrom_set_controller2(&machine.bus,0u);
        do {
            if(steps++>=MAX_STEPS_PER_FRAME) {
                printf("SMB360 CANONICAL XEX: frame timeout at %u\n",frame);
                return 5;
            }
            if(!smb360_machine_step(&machine)) {
                printf("SMB360 CANONICAL XEX: CPU stop at %04x opcode %02x\n",
                       (unsigned)machine.cpu.opcode_pc,
                       (unsigned)(machine.cpu.stopped&255));
                return 4;
            }
        }while(machine.ppu.frame<frame || machine.ppu.scanline<241u ||
               (machine.ppu.scanline==241u && machine.ppu.dot<1u));
        if(frame==0u) first_hash=fnv1a(machine.bus.completed_pixels,256u*240u);
        if(frame==0u || frame==59u || frame==119u) {
            printf("SMB360 CANONICAL XEX: frame=%u PC=%04x CPU=%llu pixels_fnv=%08x\n",
                   frame,(unsigned)machine.cpu.pc,
                   (unsigned long long)machine.cpu.cycles,
                   (unsigned)fnv1a(machine.bus.completed_pixels,256u*240u));
        }
    }
    printf("SMB360 CANONICAL XEX: probe PASS, first_pixel_hash=%08x\n",
           (unsigned)first_hash);
    /* No direct scanout framebuffer writes or unverified audio API.
     * Xbox boot, console output and clean return require physical test. */
    return 0;
}
