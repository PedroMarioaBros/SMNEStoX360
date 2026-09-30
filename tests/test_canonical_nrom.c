#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "../src/canonical/nrom.h"

int main(void) {
    uint8_t prg[SMB360_PRG_SIZE], chr[SMB360_CHR_SIZE];
    smb360_nrom m;
    size_t i;
    for (i=0;i<sizeof(prg);++i) prg[i]=(uint8_t)i;
    memset(chr,0,sizeof(chr));
    smb360_nrom_init(&m,prg,chr);

    m.ram[0]=0x11;
    assert(smb360_nrom_read(&m,0x0000)==0x11);
    assert(smb360_nrom_read(&m,0x0800)==0x11);
    smb360_nrom_write(&m,0x17ff,0x5a);
    assert(m.ram[0x7ff]==0x5a);

    assert(smb360_nrom_read(&m,0x8000)==prg[0]);
    assert(smb360_nrom_read(&m,0xffff)==prg[0x7fff]);
    smb360_nrom_write(&m,0x8000,0xff);
    assert(smb360_nrom_read(&m,0x8000)==prg[0]);

    smb360_nrom_set_controller1(&m,0xA5);
    smb360_nrom_write(&m,0x4016,1);
    smb360_nrom_write(&m,0x4016,0);
    assert(smb360_nrom_read(&m,0x4016)==1);
    assert(smb360_nrom_read(&m,0x4016)==0);
    assert(smb360_nrom_read(&m,0x4016)==1);

    puts("canonical NROM bus: PASS");
    return 0;
}
