#!/usr/bin/env python3
"""Convert a canonical 256x240 NES-index frame to PNG using only the stdlib.

Fixed illustrative RGB palette; not a simulation of NTSC analog color or emphasis.
Raw frame hashes, not RGB hashes, are the fidelity comparison input.
"""
import argparse
import hashlib
from pathlib import Path
import struct
import zlib

PALETTE = bytes.fromhex('''
666666 002a88 1412a7 3b00a4 5c007e 6e0040 6c0600 561d00
333500 0b4800 005200 004f08 00404d 000000 000000 000000
adadad 155fd9 4240ff 7527fe a01acc b71e7b b53120 994e00
6b6d00 388700 0c9300 008f32 007c8d 000000 000000 000000
ffffff 64b0ff 9290ff c676ff f36aff fe6ecc fe8170 ea9e22
bcbe00 88d800 5ce430 45e082 48cdde 4f4f4f 000000 000000
ffffff c0dfff d3d2ff e8c8ff fbc2ff fec4ea feccc5 f7d8a5
e4e594 cfef96 bdf4ab b3f3cc b5ebf2 b8b8b8 000000 000000
''')


def chunk(kind, data):
    return (struct.pack('>I', len(data)) + kind + data
            + struct.pack('>I', zlib.crc32(kind + data)))


def encode_frame(raw):
    if len(raw) != 256 * 240:
        raise ValueError('frame must contain exactly 61440 palette indices')
    if max(raw) > 63:
        raise ValueError('frame contains a palette index outside 0..63')
    rows = b''.join(b'\0' + raw[y * 256:(y + 1) * 256] for y in range(240))
    return (b'\x89PNG\r\n\x1a\n'
            + chunk(b'IHDR', struct.pack('>IIBBBBB', 256, 240, 8, 3, 0, 0, 0))
            + chunk(b'PLTE', PALETTE)
            + chunk(b'IDAT', zlib.compress(rows, 9))
            + chunk(b'IEND', b''))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('frame', type=Path)
    parser.add_argument('png', type=Path)
    args = parser.parse_args()
    raw = args.frame.read_bytes()
    png = encode_frame(raw)
    args.png.parent.mkdir(parents=True, exist_ok=True)
    args.png.write_bytes(png)
    print('frame_sha256=' + hashlib.sha256(raw).hexdigest())
    print('png=' + str(args.png))


if __name__ == '__main__':
    main()
