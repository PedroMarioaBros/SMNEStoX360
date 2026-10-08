#!/usr/bin/env python3
"""SDL smoke in an empty directory: embedded ROM, queued keys, renderer readback."""
import hashlib
import os
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
binary = ROOT / 'build/host/smb-v026-host'
# Independent binjnes S006 capture 599, scripted VBlank policy (not frame-start).
EXPECTED = 'fbde38b3940b02a1515202b5ab5ad36f828fc46bd5c5a7296bd1c1aa88f05bfc'
with tempfile.TemporaryDirectory() as directory:
    env = dict(os.environ, SDL_VIDEODRIVER='dummy')
    result = subprocess.run([str(binary), '--smoke', '--dump', 'frame.bin'],
                            cwd=directory, env=env, text=True, capture_output=True, timeout=30)
    print(result.stdout, end='')
    if result.returncode:
        raise SystemExit(result.stderr or 'SDL smoke failed')
    if 'SDL rendered pixel readback: 61440 PASS' not in result.stdout:
        raise SystemExit('renderer readback missing')
    raw = (Path(directory) / 'frame.bin').read_bytes()
    if len(raw) != 61440 or hashlib.sha256(raw).hexdigest() != EXPECTED:
        raise SystemExit('SDL input/frame mismatch against independent reference')
    if sorted(p.name for p in Path(directory).iterdir()) != ['frame.bin']:
        raise SystemExit('unexpected runtime file output')
    print('Empty working directory (no ROM file): PASS')
    print('SDL keyboard + frame 599 vs binjnes: PASS ' + EXPECTED)
