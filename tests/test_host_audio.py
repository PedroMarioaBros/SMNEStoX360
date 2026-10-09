#!/usr/bin/env python3
"""Verify deterministic NES PCM WAV export, with the user's embedded canonical ROM."""
import hashlib
import os
from pathlib import Path
import re
import shutil
import struct
import subprocess
import tempfile
import wave

ROOT = Path(__file__).resolve().parents[1]
EXE = ROOT / "build/host/smb-v026-host"
env = dict(os.environ, SDL_VIDEODRIVER="dummy", SDL_AUDIODRIVER="dummy")
digests = []
with tempfile.TemporaryDirectory() as directory:
    for run in range(2):
        name = f"capture{run}.wav"
        p = subprocess.run(
            [str(EXE), "--smoke", "--wav", name],
            cwd=directory, env=env, text=True, capture_output=True, timeout=55
        )
        if p.returncode:
            raise SystemExit(p.stderr or f"WAV smoke run {run} failed")
        match = re.search(r"audio_samples=(\d+) rate=48000 bits=16 channels=1", p.stdout)
        if not match:
            raise SystemExit("PCM sample count missing from host result")
        path = Path(directory) / name
        raw = path.read_bytes()
        with wave.open(str(path), "rb") as wav:
            if (wav.getnchannels(), wav.getsampwidth(), wav.getframerate()) != (1, 2, 48000):
                raise SystemExit("wrong WAV PCM header")
            n = wav.getnframes()
            data = wav.readframes(n)
        if n != int(match.group(1)) or len(raw) != 44 + n * 2:
            raise SystemExit("WAV header/data/sample count mismatch")
        if not 470000 < n < 490000:
            raise SystemExit(f"unexpected samples for 600 frames: {n}")
        samples = struct.unpack("<" + str(n) + "h", data)
        lo, hi = min(samples), max(samples)
        if not (-32768 < lo < 0 < hi < 32767):
            raise SystemExit(f"WAV unexpectedly silent or clipping: {lo} {hi}")
        nonzero = sum(bool(s) for s in samples)
        if nonzero < 1000:
            raise SystemExit(f"WAV too few nonzero samples: {nonzero}")
        digest = hashlib.sha256(raw).hexdigest()
        digests.append(digest)
        print(f"PCM run={run} frames={n} min={lo} max={hi} nonzero={nonzero} sha256={digest}")
    if digests[0] != digests[1]:
        raise SystemExit("WAV capture not deterministic across identical runs")
    artifact = ROOT / "build/host/canonical-600frames.wav"
    shutil.copyfile(Path(directory) / "capture0.wav", artifact)
    print("APU PCM canonical ROM WAV deterministic/no-clipping: PASS")
    print("Saved reproducible WAV artifact:", artifact.relative_to(ROOT))
