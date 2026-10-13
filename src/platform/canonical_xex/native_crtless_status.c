/* S018: observable native PPC diagnostic based on S017 canonical XEX.
 * Hardware evidence 2026-10-13:
 *   S017 01: freezes on black screen (not successful).
 *   S017 02: black screen, auto-relaunches Aurora; CPU completion UNKNOWN.
 * We make that distinction observable through XAM dashboard notifications.
 * NO unsafe video registers, CRT, Newlib, nathsou, disk writes or exploit API.
 * Notifications may be invisible under some dashboards, so no toast does not
 * prove CPU failure or boot failure.  Real Xbox validation still required.
 */
#include <stddef.h>
#include <stdint.h>
#include <xecore/xboxkrnl_threads.h>
#include <xecore/xam_loader.h>
#include "../../canonical/machine.h"

#define ROM_SIZE 40976u
#define FRAME_COUNT 120u
#define INSTRUCTION_WATCHDOG 1000000u

/* Xam public export 656: cross-checked with FreeStyle dashboard and
 * Xenia XAM API. Unlike S015, no Newlib printf/file I/O is linked. */
extern void XNotifyQueueUI(uint32_t type, uint32_t user,
                           uint64_t area, const uint16_t *display_text,
                           void *context);
extern const uint8_t smb360_canonical_rom[];
extern const uint8_t smb360_canonical_rom_end[];
static smb360_machine machine;
/* SynthXEX mapper doesn't handle .data rawSize=0 / realloc(ptr,0).
 * This initialized marker guarantees that section's raw data isn't empty. */
static volatile uint32_t s018_marker = 0x53303138u;

static const uint16_t text_started[] = {'S', 'M', 'B', '3', '6', '0', ' ', 'S', '0', '1', '8', ' ', 'C', 'P', 'U', ' ', 'S', 'T', 'A', 'R', 'T', 0};
static const uint16_t text_rom_error[] = {'S', 'M', 'B', '3', '6', '0', ' ', 'S', '0', '1', '8', ' ', 'R', 'O', 'M', ' ', 'I', 'N', 'V', 'A', 'L', 'I', 'D', 0};
static const uint16_t text_cpu_error[] = {'S', 'M', 'B', '3', '6', '0', ' ', 'S', '0', '1', '8', ' ', 'C', 'P', 'U', ' ', 'S', 'T', 'O', 'P', 0};
static const uint16_t text_timeout[] = {'S', 'M', 'B', '3', '6', '0', ' ', 'S', '0', '1', '8', ' ', 'F', 'R', 'A', 'M', 'E', ' ', 'T', 'I', 'M', 'E', 'O', 'U', 'T', 0};
static const uint16_t text_passed[] = {'S', 'M', 'B', '3', '6', '0', ' ', 'S', '0', '1', '8', ' ', '1', '2', '0', ' ', 'F', 'R', 'A', 'M', 'E', 'S', ' ', 'P', 'A', 'S', 'S', 0};

static void notify(const uint16_t *text) {
    /* Freestyle's proven homebrew call uses notification type 0x22,
     * user 0xff (all users), position/system area 1. */
    XNotifyQueueUI(0x22u, 0xffu, 1ull, text, 0);
}
static void pause_seconds(uint32_t seconds) {
    int64_t delay = -(int64_t)seconds * 10000000ll;
    (void)KeDelayExecutionThread(0u, 0u, &delay);
}
static int rom_ok(void) {
    const uint8_t *r=smb360_canonical_rom;
    return s018_marker==0x53303138u &&
        (size_t)(smb360_canonical_rom_end-r)==ROM_SIZE &&
        r[0]==0x4eu && r[1]==0x45u &&
        r[2]==0x53u && r[3]==0x1au &&
        r[4]==2u && r[5]==1u;
}
__attribute__((noreturn)) void smb360_entry(void) {
    uint32_t frame;
    int result=0;
    notify(text_started);
    /* Leave a visible opportunity to observe START before CPU work. */
    pause_seconds(2u);
    if(!rom_ok()) {
        notify(text_rom_error);
        result=3;
        goto exit_title;
    }
    smb360_machine_init(&machine,smb360_canonical_rom+16u,
                        smb360_canonical_rom+16u+SMB360_PRG_SIZE);

    for(frame=0u;frame<FRAME_COUNT;frame++) {
        uint32_t instructions=0u;
        uint64_t start_ppu_frame=machine.ppu.frame;
        /* Do not use a scanline-only condition: insist the PPU frame
         * counter actually advances 120 times. Every loop bounded. */
        do {
            if(instructions++>=INSTRUCTION_WATCHDOG) {
                notify(text_timeout);
                result=5;
                goto exit_title;
            }
            if(!smb360_machine_step(&machine)) {
                notify(text_cpu_error);
                result=4;
                goto exit_title;
            }
        } while(machine.ppu.frame==start_ppu_frame);
    }
    notify(text_passed);
exit_title:
    /* Keep the final status visible for 5 seconds, if dashboard permits.
     * XamLoaderTerminateTitle may end the title without returning to us. */
    pause_seconds(5u);
    XamLoaderTerminateTitle();
    (void)ExTerminateThread((uint32_t)result);
    /* If even the fallback returns, no documented safe destination exists;
     * unlike S017 we do not claim this is a successful return to Aurora. */
    for (;;) {}
}
