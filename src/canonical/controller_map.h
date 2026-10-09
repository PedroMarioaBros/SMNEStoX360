#ifndef SMB360_CANONICAL_CONTROLLER_MAP_H
#define SMB360_CANONICAL_CONTROLLER_MAP_H
#include <stdint.h>

/* Raw XInput wButtons values from Xbox 360 XamInputGetState.
 * Converts to the NES 8-button shift-register order: A B Sel Start U D L R. */
uint8_t smb360_xbox_buttons_to_nes(uint16_t buttons);

#endif
