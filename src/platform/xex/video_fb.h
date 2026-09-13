#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct xex_fb {
    volatile uint32_t *pixels;
    uint32_t width;
    uint32_t height;
    uint32_t tiled_width;
    uint32_t format_10bit;
    uint32_t cleared;
};

int smb360_xex_fb_open(struct xex_fb *out);
int smb360_xex_fb_present_rgb888(struct xex_fb *fb,
                                 const uint8_t *rgb,
                                 uint32_t src_w,
                                 uint32_t src_h);

#ifdef __cplusplus
}
#endif
