#ifndef SMB360_CANONICAL_NROM_H
#define SMB360_CANONICAL_NROM_H
#include <stddef.h>
#include <stdint.h>

#define SMB360_PRG_SIZE 32768u
#define SMB360_CHR_SIZE 8192u

typedef struct {
    uint8_t ram[0x800];
    uint8_t ppu_regs[8];
    uint8_t apu_io[0x18];
    const uint8_t *prg;
    const uint8_t *chr;
    uint8_t controller1;
    uint8_t controller_shift;
    uint8_t controller2, controller2_shift;
    uint8_t controller_strobe;
    uint8_t ppu_status;
    uint8_t ppu_write_latch;
    uint8_t nmi_line, nmi_pending;
    uint8_t nametable[0x800], palette[32], oam[256];
    uint16_t ppu_v, ppu_t;
    uint8_t fine_x, ppu_read_buffer, ppu_open_bus, oam_addr;
    uint8_t dma_pending, dma_page;
    uint16_t line_v;
    uint8_t sprite_indices[8], line_sprites;
    uint8_t pixels[256*240], completed_pixels[256*240];
    uint64_t sprite_zero_hits;
} smb360_nrom;

void smb360_nrom_init(smb360_nrom *m, const uint8_t *prg, const uint8_t *chr);
uint8_t smb360_nrom_read(smb360_nrom *m, uint16_t addr);
void smb360_nrom_write(smb360_nrom *m, uint16_t addr, uint8_t value);
void smb360_nrom_set_controller1(smb360_nrom *m, uint8_t buttons);
void smb360_nrom_set_controller2(smb360_nrom *m, uint8_t buttons);
void smb360_nrom_set_vblank(smb360_nrom *m, int active);

uint8_t smb360_nrom_ppu_read(const smb360_nrom *m, uint16_t addr);
void smb360_nrom_ppu_write(smb360_nrom *m, uint16_t addr, uint8_t value);

#endif
