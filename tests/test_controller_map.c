#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "../src/canonical/controller_map.h"
#include "../src/canonical/nrom.h"

static void test_mapping(void) {
    const uint16_t raw[]={0x1000u,0x2000u,0x0020u,0x0010u,
                          0x0001u,0x0002u,0x0004u,0x0008u};
    unsigned i;
    assert(smb360_xbox_buttons_to_nes(0u)==0u);
    for(i=0u;i<8u;i++)
        assert(smb360_xbox_buttons_to_nes(raw[i])==(uint8_t)(1u<<i));
    assert(smb360_xbox_buttons_to_nes(0x303fu)==0x0fu);
    assert(smb360_xbox_buttons_to_nes(0xffffu)==0x0fu);
    assert(smb360_xbox_buttons_to_nes(0x1008u)==0x81u);
}
static void test_original_nrom_controller_bus(void) {
    uint8_t prg[32768]={0},chr[8192]={0};
    smb360_nrom b;
    unsigned i;
    smb360_nrom_init(&b,prg,chr);
    smb360_nrom_set_controller1(&b,smb360_xbox_buttons_to_nes(0x1008u));
    smb360_nrom_set_controller2(&b,smb360_xbox_buttons_to_nes(0x2010u));
    smb360_nrom_write(&b,0x4016u,1u);
    smb360_nrom_write(&b,0x4016u,0u);
    for(i=0;i<8u;i++) {
        assert((smb360_nrom_read(&b,0x4016u)&1u)==((0x81u>>i)&1u));
        assert((smb360_nrom_read(&b,0x4017u)&1u)==((0x0au>>i)&1u));
    }
}
int main(void) {
    test_mapping();
    test_original_nrom_controller_bus();
    puts("Xbox pad to NES controller1/controller2 bus mapping: PASS");
    return 0;
}
