#!/usr/bin/env python3
"""Build SDL2 host bench with canonical PRG/CHR embedded after fingerprint checks."""
import argparse
import hashlib
import os
from pathlib import Path
import shlex
import subprocess
from extract_canonical_rom import EXPECTED_ROM_SHA256, EXPECTED_PRG_SHA256, EXPECTED_CHR_SHA256
from frame_to_png import PALETTE
ROOT = Path(__file__).resolve().parents[1]


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--sdl-include', type=Path, help='SDL2 headers, if sdl2-config is unavailable')
    ap.add_argument('--sdl-library', type=Path, help='SDL2 shared library, used with --sdl-include')
    args = ap.parse_args()
    if bool(args.sdl_include) != bool(args.sdl_library):
        ap.error('both SDL override paths are required together')
    raw = (ROOT / 'assets/canonical/SMB_v026.nes').read_bytes()
    prg, chr_ = raw[16:32784], raw[32784:]
    for data, expected in [(raw, EXPECTED_ROM_SHA256), (prg, EXPECTED_PRG_SHA256), (chr_, EXPECTED_CHR_SHA256)]:
        if hashlib.sha256(data).hexdigest() != expected:
            raise SystemExit('canonical fingerprint mismatch; build refused')
    out = ROOT / 'build/host'
    out.mkdir(parents=True, exist_ok=True)
    text = '#include <stdint.h>\n'
    for name, data in [('canonical_prg', prg), ('canonical_chr', chr_)]:
        text += 'static const uint8_t ' + name + '[] = {\n'
        text += '\n'.join(','.join(str(v) for v in data[i:i+32]) + ',' for i in range(0, len(data), 32))
        text += '\n};\n'
    text += 'static const uint32_t host_palette[64] = {\n'
    text += ','.join('0xff' + PALETTE[i:i+3].hex() + 'u' for i in range(0,192,3)) + '\n};\n'
    (out / 'canonical_assets.h').write_text(text)
    if args.sdl_include:
        sdl = ['-I' + str(args.sdl_include.resolve()), str(args.sdl_library.resolve())]
    else:
        sdl = shlex.split(subprocess.check_output(['sdl2-config', '--cflags', '--libs'], text=True))
    command = [os.environ.get('CC', 'cc'), '-std=c99', '-O2', '-Wall', '-Wextra', '-Werror',
               '-I' + str(out), *map(str, sorted((ROOT/'src/canonical').glob('*.c'))),
               str(ROOT/'src/host/main.c'), *sdl, '-o', str(out/'smb-v026-host')]
    subprocess.run(command, check=True)
    print('ROM SHA-256 verified:', EXPECTED_ROM_SHA256)
    print('Built:', out/'smb-v026-host')


if __name__ == '__main__':
    main()
