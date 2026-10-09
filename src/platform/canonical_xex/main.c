/* S014: CANONICAL ROM diagnostic only. No unsafe framebuffer addresses.
 * Executes PRG/CHR from SMB_v026.nes through the canonical C99 core.
 * Not a playable Xbox port: output currently goes to diagnostic stdout.
 * CPU6502 is still interpreted in C; no native PPC translation is claimed. */
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <xecore/xboxkrnl.h>
#include "../../canonical/machine.h"
#include "../../canonical/controller_map.h"
#include <string.h>

#define NES_ROM_BYTES 40976u
#define PROBE_FRAMES 120u
/* Fixed run horizon; game loop automatically exits after 120 VBlank captures.
 * No undocumented framebuffer writes or indefinite console lock-up. */
#define TICKS_100NS_PER_FRAME 166667
#define NOTIFICATION_HOLD_100NS 40000000
#define MAX_STEPS_PER_FRAME 1000000u

extern const uint8_t smb360_canonical_rom[];
extern const uint8_t smb360_canonical_rom_end[];

static smb360_machine machine;

/* Public XAM exports listed in OpenXeChain/xecorelib xam.def:
 * XamInputGetState @401 and XNotifyQueueUI @656.
 * Avoid wchar_t: Xbox 360 XAM strings are 16-bit UTF-16 units. */
typedef struct {
    uint16_t buttons;
    uint8_t left_trigger, right_trigger;
    int16_t thumb_lx, thumb_ly, thumb_rx, thumb_ry;
} xpad_gamepad;
typedef struct {
    uint32_t packet_number;
    xpad_gamepad gamepad;
} xpad_state;
typedef char xpad_layout_check[(sizeof(xpad_state)==16u)?1:-1];

extern uint32_t XamInputGetState(uint32_t user, uint32_t flags,
                                 xpad_state *state);
extern uint32_t XNotifyQueueUI(uint32_t kind, uint32_t user, uint64_t area,
                                const uint16_t *message, void *context);

static uint16_t notification_text[96];

static void notify(const char *ascii) {
    size_t i=0u;
    /* Static buffer survives asynchronous notification processing. */
    while(ascii[i] && i+1u<sizeof(notification_text)/sizeof(notification_text[0])) {
        notification_text[i]=(uint16_t)(uint8_t)ascii[i];
        ++i;
    }
    notification_text[i]=0u;
    (void)XNotifyQueueUI(14u,0u,2ull,notification_text,0);
}

static void delay_100ns(int64_t amount) {
    int64_t relative=-amount;
    KeDelayExecutionThread(0u,0u,&relative);
}

static uint8_t sample_pad(unsigned port) {
    xpad_state s;
    memset(&s,0,sizeof(s));
    if(XamInputGetState((uint32_t)port,0u,&s)!=0u)return 0u;
    return smb360_xbox_buttons_to_nes(s.gamepad.buttons);
}


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
    printf("SMB360 CANONICAL XEX S015 diagnostic: startup\n");
    notify("SMB360 S015 START - 120 frames");
    if(!check_embedded_rom()) {
        printf("SMB360 CANONICAL XEX: embedded iNES header/size INVALID\n");
        notify("SMB360 S015 ROM INVALID");
        delay_100ns(NOTIFICATION_HOLD_100NS);
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
        smb360_nrom_set_controller1(&machine.bus,sample_pad(0u));
        smb360_nrom_set_controller2(&machine.bus,sample_pad(1u));
        do {
            if(steps++>=MAX_STEPS_PER_FRAME) {
                printf("SMB360 CANONICAL XEX: frame timeout at %u\n",frame);
                notify("SMB360 S015 FRAME TIMEOUT");
                delay_100ns(NOTIFICATION_HOLD_100NS);
                return 5;
            }
            if(!smb360_machine_step(&machine)) {
                printf("SMB360 CANONICAL XEX: CPU stop at %04x opcode %02x\n",
                       (unsigned)machine.cpu.opcode_pc,
                       (unsigned)(machine.cpu.stopped&255));
                notify("SMB360 S015 CPU STOP");
                delay_100ns(NOTIFICATION_HOLD_100NS);
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
        delay_100ns(TICKS_100NS_PER_FRAME);
    }
    printf("SMB360 CANONICAL XEX: probe PASS, first_pixel_hash=%08x\n",
           (unsigned)first_hash);
    notify("SMB360 S015 120 FRAMES PASS");
    delay_100ns(NOTIFICATION_HOLD_100NS);
    /* The dashboard UI notification is a system service, not a GPU renderer.
     * Console execution and visible toast STILL require real hardware test.
     * No raw scanout writes, audio, or persistent background thread. */
    return 0;
}
