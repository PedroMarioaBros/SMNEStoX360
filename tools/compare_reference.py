#!/usr/bin/env python3
"""Reproduce canonical-ROM differential captures against pinned MIT binjnes.

Host validation only. No reference code is linked into the game runtime.
Inputs are changed immediately after the previous VBlank capture, before the
next NMI controller poll. This is NOT the frame-start policy of run_canonical.py.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import tarfile

from extract_canonical_rom import EXPECTED_ROM_SHA256, EXPECTED_PRG_SHA256, EXPECTED_CHR_SHA256

ROOT = Path(__file__).resolve().parents[1]
PIN = 'e2f5871a28ff189daa82b70be9324a42e2aaf9fd'


def digest(path):
    h = hashlib.sha256()
    with path.open('rb') as f:
        for block in iter(lambda: f.read(1024 * 1024), b''):
            h.update(block)
    return h.hexdigest()


def compare(a, b, size, count):
    if a.stat().st_size != size * count or b.stat().st_size != size * count:
        raise ValueError('capture size mismatch: ' + str(a))
    different = []
    first = None
    with a.open('rb') as fa, b.open('rb') as fb:
        for frame in range(count):
            x, y = fa.read(size), fb.read(size)
            if x != y:
                offsets = [i for i, (u, v) in enumerate(zip(x, y)) if u != v]
                different.append(frame)
                if first is None:
                    first = {'capture': frame, 'different_bytes': len(offsets),
                             'first_offsets': [{'offset': i, 'canonical': x[i], 'reference': y[i]}
                                               for i in offsets[:16]]}
    return {'captures': count, 'equal_captures': count - len(different),
            'different_captures': len(different), 'first_difference': first,
            'canonical_sha256': digest(a), 'reference_sha256': digest(b)}


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--output', type=Path, default=ROOT / 'build/reference')
    ap.add_argument('--idle-frames', type=int, default=600)
    ap.add_argument('--input-frames', type=int, default=6000)
    args = ap.parse_args()
    if not (1 <= args.idle_frames <= 10000 and 1 <= args.input_frames <= 10000):
        ap.error('frame counts must be 1..10000')
    out = args.output.resolve()
    out.mkdir(parents=True, exist_ok=True)
    rom = ROOT / 'assets/canonical/SMB_v026.nes'
    raw = rom.read_bytes()
    for data, expected in [(raw, EXPECTED_ROM_SHA256), (raw[16:32784], EXPECTED_PRG_SHA256),
                           (raw[32784:], EXPECTED_CHR_SHA256)]:
        if hashlib.sha256(data).hexdigest() != expected:
            raise ValueError('canonical ROM/PRG/CHR fingerprint mismatch')
    vendor = ROOT / 'tests/reference/vendor'
    meta = json.loads((vendor / 'manifest.json').read_text())
    archive = vendor / 'binjnes-e2f5871.tar.xz'
    if meta['commit'] != PIN or digest(archive) != meta['archive_sha256']:
        raise ValueError('reference archive provenance mismatch')
    ref = out / 'binjnes'
    with tarfile.open(archive, 'r:xz') as tar:
        if set(tar.getnames()) != set(meta['files']):
            raise ValueError('unexpected reference source files')
        for member in tar.getmembers():
            if not member.isfile() or Path(member.name).is_absolute() or '..' in Path(member.name).parts:
                raise ValueError('invalid reference archive member')
            data = tar.extractfile(member).read()
            if hashlib.sha256(data).hexdigest() != meta['files'][member.name]:
                raise ValueError('reference member hash mismatch')
            target = ref / member.name
            target.parent.mkdir(parents=True, exist_ok=True)
            target.write_bytes(data)
    cc = os.environ.get('CC', 'cc')
    flags = ['-O2', '-Wall', '-Wextra', '-Werror']
    adapter = ROOT / 'tests/reference/capture.c'
    own = [cc, '-std=c99', *flags, *map(str, sorted((ROOT / 'src/canonical').glob('*.c'))),
           str(adapter), '-o', str(out / 'canonical')]
    reference = [cc, '-std=gnu11', *flags, '-Wno-error=format-security',
                 '-Wno-unused-parameter', '-Wno-unused-function', '-Wno-unused-variable',
                 '-Wno-implicit-fallthrough', '-Wno-unknown-pragmas',
                 '-DBINJNES_GCC', '-DBINJNES_THREAD_C11', '-DBINJNES_ATOMIC_C11',
                 '-DREFERENCE_BINJNES', '-I' + str(ref / 'src'),
                 *[str(ref / 'src' / s) for s in ['common.c', 'cartdb.c', 'emulator.c']],
                 str(adapter), '-pthread', '-lm', '-o', str(out / 'reference')]
    with (out / 'compile.log').open('w') as log:
        for command in [own, reference]:
            subprocess.run(command, check=True, stdout=log, stderr=log, timeout=120)
    report = {'reference_commit': PIN, 'rom_sha256': digest(rom),
              'capture_policy': 'zero-based VBlank sequence; input set after previous capture',
              'pixel_format': '61440 six-bit palette indices; emphasis rejected',
              'ram_format': 'all 2048 CPU RAM bytes; no exclusions', 'scenarios': {}}
    for name, count, scripted in [('idle', args.idle_frames, 0), ('input', args.input_frames, 1)]:
        for engine in ['canonical', 'reference']:
            prefix = out / (name + '-' + engine)
            with prefix.with_suffix('.csv').open('w') as log:
                subprocess.run([str(out / engine), str(rom), str(prefix) + '.frames',
                                str(prefix) + '.ram', str(count), str(scripted)],
                               check=True, stdout=log, timeout=120)
        result = {}
        for kind, size in [('frames', 61440), ('ram', 2048)]:
            result[kind] = compare(out / (name + '-canonical.' + kind),
                                   out / (name + '-reference.' + kind), size, count)
        report['scenarios'][name] = result
        print(name, 'pixels:', result['frames']['equal_captures'], '/', count,
              'RAM:', result['ram']['equal_captures'], '/', count, flush=True)
    (out / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
    if any(r[k]['different_captures'] for r in report['scenarios'].values() for k in ['frames', 'ram']):
        raise SystemExit(1)


if __name__ == '__main__':
    main()
