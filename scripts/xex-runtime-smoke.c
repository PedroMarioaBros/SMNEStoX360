#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <xecore/xboxkrnl.h>

/* xam.xex ordinal 401; xecorelib currently exports the stub even though
 * this prototype is not yet provided by its public xam headers. */
typedef struct {
    uint16_t buttons;
    uint8_t left_trigger;
    uint8_t right_trigger;
    int16_t thumb_lx;
    int16_t thumb_ly;
    int16_t thumb_rx;
    int16_t thumb_ry;
} smb360_xinput_gamepad;

typedef struct {
    uint32_t packet_number;
    smb360_xinput_gamepad gamepad;
} smb360_xinput_state;

extern uint32_t XamInputGetState(uint32_t user_index, uint32_t flags,
                                 smb360_xinput_state *state);

static int validate_rom_header(const uint8_t *h, size_t n) {
    if (!h || n < 16) return 0;
    if (h[0] != 'N' || h[1] != 'E' || h[2] != 'S' || h[3] != 0x1a) return 0;
    if (h[4] != 2 || h[5] != 1) return 0; /* 32 KiB PRG, 8 KiB CHR */
    if (((h[6] >> 4) | (h[7] & 0xf0)) != 0) return 0; /* mapper 0 */
    if (h[6] & 0x04) return 0; /* no trainer */
    return 1;
}

static FILE *open_owner_rom(void) {
    static const char *paths[] = {
        "game:\\smb.nes",
        "smb.nes",
        "usb0:\\smb.nes",
        "usb1:\\smb.nes"
    };
    unsigned i;
    for (i = 0; i < sizeof(paths) / sizeof(paths[0]); ++i) {
        FILE *f = fopen(paths[i], "rb");
        if (f) {
            printf("SMB360 XEX: ROM opened at %s\n", paths[i]);
            return f;
        }
    }
    return NULL;
}

int main(void) {
    uint8_t header[16];
    FILE *f;
    smb360_xinput_state pad;

    printf("SMB360 XEX runtime smoke starting\n");
    f = open_owner_rom();
    if (!f) {
        printf("SMB360 XEX: smb.nes not found\n");
        return 2;
    }
    if (fread(header, 1, sizeof(header), f) != sizeof(header)) {
        fclose(f);
        printf("SMB360 XEX: ROM header read failed\n");
        return 3;
    }
    fclose(f);
    if (!validate_rom_header(header, sizeof(header))) {
        printf("SMB360 XEX: ROM geometry/mapper check failed\n");
        return 4;
    }
    printf("SMB360 XEX: owner ROM geometry OK\n");

    memset(&pad, 0, sizeof(pad));
    if (XamInputGetState(0, 0, &pad) == 0) {
        printf("SMB360 XEX: controller 1 available, buttons=0x%04x\n",
               (unsigned)pad.gamepad.buttons);
    } else {
        printf("SMB360 XEX: controller 1 not available at startup\n");
    }

    /* Keep the title alive without consuming an entire CPU while the runtime
     * smoke binary is inspected on hardware. Negative interval is relative,
     * in 100 ns units. */
    for (;;) {
        int64_t interval = -10000000LL;
        KeDelayExecutionThread(0, 0, &interval);
    }
}
