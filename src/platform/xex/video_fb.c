#include "video_fb.h"
#include <stddef.h>
#include <xecore/xboxkrnl.h>

/* Dashboard/Xbox-OS framebuffer discovery used by open Xbox 360 homebrew.
 * VdGetCurrentDisplayInformation fills an 0x58-byte structure whose width
 * and height are at +0x10/+0x14. The scanout surface itself is tiled 32x32.
 * Keep all address knowledge isolated here so a future D3D/OpenXeChain
 * backend can replace this implementation without touching the game core. */

static inline uint32_t tiled_index(uint32_t cw, uint32_t x, uint32_t y) {
    return (((y >> 5) * 32u * cw + ((x >> 5) << 10)
             + (x & 3u) + ((y & 1u) << 2)
             + (((x & 31u) >> 2) << 3)
             + (((y & 31u) >> 1) << 6))
            ^ ((y & 8u) << 2));
}

static uint32_t pack_a8r8g8b8(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)b << 24) | ((uint32_t)g << 16) |
           ((uint32_t)r << 8) | 0xffu;
}

static uint32_t pack_a2r10g10b10(uint8_t r, uint8_t g, uint8_t b) {
    uint32_t rr = ((uint32_t)r << 2) | ((uint32_t)r >> 6);
    uint32_t gg = ((uint32_t)g << 2) | ((uint32_t)g >> 6);
    uint32_t bb = ((uint32_t)b << 2) | ((uint32_t)b >> 6);
    uint32_t native = (3u << 30) | (rr << 20) | (gg << 10) | bb;
    return ((native & 0x000000ffu) << 24) |
           ((native & 0x0000ff00u) << 8) |
           ((native & 0x00ff0000u) >> 8) |
           ((native & 0xff000000u) >> 24);
}

int smb360_xex_fb_open(struct xex_fb *out) {
    uint8_t info[0x58] = {0};
    uint32_t base_offset;
    uint32_t mode_word;
    if (!out) return 0;

    VdGetCurrentDisplayInformation(info);
    out->width = *(uint32_t *)(void *)(info + 0x10);
    out->height = *(uint32_t *)(void *)(info + 0x14);
    if (out->width < 320u || out->height < 240u ||
        out->width > 1920u || out->height > 1080u) return 0;

    base_offset = *(volatile uint32_t *)(uintptr_t)0x7fc86110u;
    mode_word = *(volatile uint32_t *)(uintptr_t)0x7fc86104u;
    out->pixels = (volatile uint32_t *)(uintptr_t)(0xdffff000u + base_offset);
    out->tiled_width = (out->width + 31u) & ~31u;
    out->format_10bit = (((mode_word >> 24) & 7u) != 0u);
    out->cleared = 0u;
    return out->pixels != 0;
}

int smb360_xex_fb_present_rgb888(struct xex_fb *fb,
                                 const uint8_t *rgb,
                                 uint32_t src_w,
                                 uint32_t src_h) {
    uint32_t scale, dst_w, dst_h, ox, oy;
    uint32_t x, y;
    if (!fb || !fb->pixels || !rgb || src_w == 0 || src_h == 0) return 0;

    /* Integer scaling keeps SMB pixels crisp. Cap at 3x so 1080p does not
     * multiply each NES pixel into an unnecessarily large CPU-written block. */
    scale = fb->width / src_w;
    if (fb->height / src_h < scale) scale = fb->height / src_h;
    if (scale == 0u) scale = 1u;
    if (scale > 3u) scale = 3u;
    dst_w = src_w * scale;
    dst_h = src_h * scale;
    ox = (fb->width - dst_w) / 2u;
    oy = (fb->height - dst_h) / 2u;

    /* Clear the background once. Subsequent frames only touch the centered
     * game rectangle, reducing 1080p CPU stores by roughly an order of magnitude. */
    if (!fb->cleared) {
        for (y = 0; y < fb->height; ++y) {
            for (x = 0; x < fb->width; ++x)
                fb->pixels[tiled_index(fb->tiled_width, x, y)] = 0u;
        }
        fb->cleared = 1u;
    }

    for (y = 0; y < src_h; ++y) {
        for (x = 0; x < src_w; ++x) {
            const uint8_t *p = rgb + ((size_t)y * src_w + x) * 3u;
            uint32_t c = fb->format_10bit ? pack_a2r10g10b10(p[0], p[1], p[2])
                                          : pack_a8r8g8b8(p[0], p[1], p[2]);
            uint32_t yy, xx;
            for (yy = 0; yy < scale; ++yy) {
                uint32_t dy = oy + y * scale + yy;
                for (xx = 0; xx < scale; ++xx) {
                    uint32_t dx = ox + x * scale + xx;
                    fb->pixels[tiled_index(fb->tiled_width, dx, dy)] = c;
                }
            }
        }
    }

    __asm__ volatile("sync" ::: "memory");
    return 1;
}
