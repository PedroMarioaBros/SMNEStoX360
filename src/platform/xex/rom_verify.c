#include "rom_verify.h"
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#define SMB360_ROM_BYTES 40976u
#define SHA1_BLOCK 64u

static uint8_t g_rom[SMB360_ROM_BYTES];

static uint32_t rol32(uint32_t x, unsigned n) {
    return (x << n) | (x >> (32u - n));
}

static uint32_t load_be32(const uint8_t *p) {
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
           ((uint32_t)p[2] << 8) | (uint32_t)p[3];
}

static void sha1_compress(uint32_t h[5], const uint8_t block[64]) {
    uint32_t w[80];
    uint32_t a, b, c, d, e;
    unsigned i;
    for (i = 0; i < 16; ++i) w[i] = load_be32(block + i * 4u);
    for (i = 16; i < 80; ++i)
        w[i] = rol32(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);
    a = h[0]; b = h[1]; c = h[2]; d = h[3]; e = h[4];
    for (i = 0; i < 80; ++i) {
        uint32_t f, k, t;
        if (i < 20) { f = (b & c) | ((~b) & d); k = 0x5a827999u; }
        else if (i < 40) { f = b ^ c ^ d; k = 0x6ed9eba1u; }
        else if (i < 60) { f = (b & c) | (b & d) | (c & d); k = 0x8f1bbcdcu; }
        else { f = b ^ c ^ d; k = 0xca62c1d6u; }
        t = rol32(a, 5) + f + e + k + w[i];
        e = d; d = c; c = rol32(b, 30); b = a; a = t;
    }
    h[0] += a; h[1] += b; h[2] += c; h[3] += d; h[4] += e;
}

static void sha1_bytes(const uint8_t *data, size_t size, uint8_t out[20]) {
    uint32_t h[5] = {0x67452301u, 0xefcdab89u, 0x98badcfeu,
                     0x10325476u, 0xc3d2e1f0u};
    uint8_t tail[128];
    size_t full = size & ~(size_t)(SHA1_BLOCK - 1u);
    size_t rem = size - full;
    size_t padded;
    uint64_t bits = (uint64_t)size * 8ull;
    size_t off;
    unsigned i;

    for (off = 0; off < full; off += SHA1_BLOCK)
        sha1_compress(h, data + off);

    memset(tail, 0, sizeof(tail));
    if (rem) memcpy(tail, data + full, rem);
    tail[rem] = 0x80u;
    padded = (rem + 1u <= 56u) ? 64u : 128u;
    for (i = 0; i < 8; ++i)
        tail[padded - 8u + i] = (uint8_t)(bits >> (56u - i * 8u));
    sha1_compress(h, tail);
    if (padded == 128u) sha1_compress(h, tail + 64u);

    for (i = 0; i < 5; ++i) {
        out[i * 4u + 0u] = (uint8_t)(h[i] >> 24);
        out[i * 4u + 1u] = (uint8_t)(h[i] >> 16);
        out[i * 4u + 2u] = (uint8_t)(h[i] >> 8);
        out[i * 4u + 3u] = (uint8_t)h[i];
    }
}

static int accepted_sha1(const uint8_t d[20]) {
    static const uint8_t known[][20] = {
        /* SMB1 World */
        {0x33,0xd2,0x3c,0x2f,0x2c,0xfa,0x4c,0x9e,0xfe,0xc8,0x7f,0x7b,0xc1,0x32,0x1c,0xe3,0xce,0x6c,0x89,0xbd},
        /* User-supplied SMB_v026 */
        {0xdc,0xee,0xf7,0xc9,0x69,0xd7,0xa7,0xbf,0x5b,0xbc,0x21,0xda,0x3e,0x95,0x61,0x87,0x65,0xb1,0xfb,0x1a},
        /* SMB1 PT-BR 1.0 BMatSantos */
        {0x8d,0xc2,0xfa,0xf0,0xae,0x79,0x4b,0x3c,0x51,0x92,0xd4,0xd6,0x69,0xd0,0x71,0x73,0xcc,0x53,0xb6,0x26}
    };
    unsigned i;
    for (i = 0; i < sizeof(known) / sizeof(known[0]); ++i)
        if (memcmp(d, known[i], 20u) == 0) return 1;
    return 0;
}

static int verify_path(const char *path) {
    FILE *f;
    uint8_t d[20];
    int extra;
    f = fopen(path, "rb");
    if (!f) return 0;
    if (fread(g_rom, 1, sizeof(g_rom), f) != sizeof(g_rom)) {
        fclose(f);
        return 0;
    }
    extra = fgetc(f);
    fclose(f);
    if (extra != EOF) return 0;
    if (g_rom[0] != 'N' || g_rom[1] != 'E' || g_rom[2] != 'S' || g_rom[3] != 0x1a ||
        g_rom[4] != 2u || g_rom[5] != 1u || (g_rom[6] & 0x04u) != 0u ||
        (((g_rom[6] >> 4) | (g_rom[7] & 0xf0u)) != 0u)) return 0;
    sha1_bytes(g_rom, sizeof(g_rom), d);
    return accepted_sha1(d);
}

const char *smb360_xex_verified_rom_path(void) {
    static const char *paths[] = {
        "game:\\smb.nes",
        "smb.nes",
        "usb0:\\smb.nes",
        "usb1:\\smb.nes"
    };
    unsigned i;
    for (i = 0; i < sizeof(paths) / sizeof(paths[0]); ++i) {
        if (verify_path(paths[i])) return paths[i];
    }
    return NULL;
}
