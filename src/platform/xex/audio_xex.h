#pragma once

#ifdef __cplusplus
extern "C" {
#endif

int smb360_xex_audio_init(void);
void smb360_xex_audio_shutdown(void);
int smb360_xex_audio_ready(void);
/* Call once per 60 Hz emulation frame, after apu_step_frame(). */
void smb360_xex_audio_produce_frame(void);

#ifdef __cplusplus
}
#endif
