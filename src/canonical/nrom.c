#include "nrom.h"
#include <string.h>

void smb360_nrom_init(smb360_nrom *m, const uint8_t *prg, const uint8_t *chr) {
    memset(m, 0, sizeof(*m));
    m->prg = prg;
    m->chr = chr;
}

void smb360_nrom_set_controller1(smb360_nrom *m, uint8_t buttons) {
    m->controller1 = buttons;
}

void smb360_nrom_set_vblank(smb360_nrom *m, int active) {
    if (active) m->ppu_status |= 0x80u;
    else m->ppu_status &= (uint8_t)~0x80u;
}

uint8_t smb360_nrom_read(smb360_nrom *m, uint16_t a) {
    if (a < 0x2000u) return m->ram[a & 0x07ffu];
    if (a < 0x4000u) {
        uint8_t r = (uint8_t)(a & 7u);
        if (r == 2u) {
            uint8_t v = m->ppu_status;
            m->ppu_status &= (uint8_t)~0x80u; /* reading PPUSTATUS clears vblank */
            m->ppu_write_latch = 0u;          /* and resets $2005/$2006 latch */
            return v;
        }
        return m->ppu_regs[r];
    }
    if (a == 0x4016u) {
        uint8_t v = (uint8_t)(m->controller_shift & 1u);
        if (!m->controller_strobe)
            m->controller_shift = (uint8_t)((m->controller_shift >> 1) | 0x80u);
        return v;
    }
    if (a >= 0x4000u && a <= 0x4017u) return m->apu_io[a - 0x4000u];
    if (a >= 0x8000u) return m->prg[a - 0x8000u]; /* NROM-256: full 32 KiB */
    return 0u; /* expansion/SRAM not used as ROM mapping */
}

void smb360_nrom_write(smb360_nrom *m, uint16_t a, uint8_t v) {
    if (a < 0x2000u) {
        m->ram[a & 0x07ffu] = v;
        return;
    }
    if (a < 0x4000u) {
        uint8_t r = (uint8_t)(a & 7u);
        m->ppu_regs[r] = v;
        if (r == 5u || r == 6u) m->ppu_write_latch ^= 1u;
        return;
    }
    if (a == 0x4016u) {
        uint8_t new_strobe = (uint8_t)(v & 1u);
        if (new_strobe || m->controller_strobe)
            m->controller_shift = m->controller1;
        m->controller_strobe = new_strobe;
        return;
    }
    if (a >= 0x4000u && a <= 0x4017u) {
        m->apu_io[a - 0x4000u] = v;
        return;
    }
    /* NROM PRG is read-only. */
}
