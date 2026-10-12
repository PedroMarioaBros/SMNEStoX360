/* S017 native PowerPC entry plus the owner's exact original SMB ROM.
 * This is a CPU/PPU hardware runtime diagnostic, NOT playable:
 * no safe Xbox graphics/audio backend exists yet.
 * It intentionally bypasses Newlib CRT and imports only 3 OS exports.
 * DO NOT run before the independent S017 boot-only variant works.
 */
#include <stddef.h>
#include <stdint.h>
#include <xecore/xboxkrnl_threads.h>
#include <xecore/xam_loader.h>
#include "../../canonical/machine.h"

#define ROM_SIZE 40976u
#define MAX_FRAMES 120u
#define FRAME_STEP_WATCHDOG 1000000u
extern const uint8_t smb360_canonical_rom[];
extern const uint8_t smb360_canonical_rom_end[];
static smb360_machine machine;
/* SynthXEX's PE mapper treats realloc(ptr,0) as ERR_OUT_OF_MEM when
 * .data has zero raw bytes (BSS-only).  Keep one intentionally
 * initialized, used scalar so .data has nonzero raw size. This is
 * NOT a gameplay patch and does not modify ROM bytes. */
static volatile uint32_t s017_data_section_marker = 0x534D4232u;

static int run_probe(void) {
    uint32_t frame;
    const uint8_t *rom=smb360_canonical_rom;
    if(s017_data_section_marker!=0x534D4232u) return 0;
    if((size_t)(smb360_canonical_rom_end-rom)!=ROM_SIZE ||
       rom[0]!=0x4eu || rom[1]!=0x45u || rom[2]!=0x53u ||
       rom[3]!=0x1au || rom[4]!=2u || rom[5]!=1u) return 0;
    smb360_machine_init(&machine,rom+16u,rom+16u+SMB360_PRG_SIZE);
    for(frame=0u;frame<MAX_FRAMES;frame++) {
        uint32_t steps=0u;
        do {
            if(steps++>=FRAME_STEP_WATCHDOG) return 0;
            if(!smb360_machine_step(&machine)) return 0;
        } while(machine.ppu.frame<frame ||
                machine.ppu.scanline<241u ||
                (machine.ppu.scanline==241u && machine.ppu.dot<1u));
    }
    return 1;
}
__attribute__((noreturn)) void smb360_entry(void) {
    int ok=run_probe();
    /* Five seconds = completed work; one second = early CPU/ROM error.
     * Approximate duration is diagnostic, NOT a real display/telemetry. */
    int64_t relative_100ns=ok?-50000000ll:-10000000ll;
    (void)KeDelayExecutionThread(0u,0u,&relative_100ns);
    XamLoaderTerminateTitle();
    (void)ExTerminateThread(ok?0u:1u);
    for (;;) {}
}
