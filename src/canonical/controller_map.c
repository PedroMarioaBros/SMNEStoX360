#include "controller_map.h"

uint8_t smb360_xbox_buttons_to_nes(uint16_t buttons) {
    uint8_t nes=0u;
    if(buttons&0x1000u)nes|=0x01u; /* Xbox A = NES A */
    if(buttons&0x2000u)nes|=0x02u; /* Xbox B = NES B */
    if(buttons&0x0020u)nes|=0x04u; /* Back = Select */
    if(buttons&0x0010u)nes|=0x08u; /* Start = Start */
    if(buttons&0x0001u)nes|=0x10u;
    if(buttons&0x0002u)nes|=0x20u;
    if(buttons&0x0004u)nes|=0x40u;
    if(buttons&0x0008u)nes|=0x80u;
    /* Preserve existing host policy: opposing directions cancel. */
    if((nes&0x30u)==0x30u)nes&=(uint8_t)~0x30u;
    if((nes&0xc0u)==0xc0u)nes&=(uint8_t)~0xc0u;
    return nes;
}
