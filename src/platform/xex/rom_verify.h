#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/* Returns the verified ROM path, or NULL if no accepted SMB1 image exists. */
const char *smb360_xex_verified_rom_path(void);

#ifdef __cplusplus
}
#endif
