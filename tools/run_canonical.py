#!/usr/bin/env python3
"""Fingerprint gate and reproducible host runner. Never downloads a ROM."""
import argparse
import hashlib
from pathlib import Path
import subprocess
from extract_canonical_rom import EXPECTED_ROM_SHA256, EXPECTED_PRG_SHA256, EXPECTED_CHR_SHA256

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("rom", type=Path)
    ap.add_argument("--runner", type=Path, default=Path("build/run_canonical_rom"))
    ap.add_argument("--instructions", type=int, default=1_000_000)
    ap.add_argument("--frame", type=Path, default=Path("build/frame.bin"))
    ap.add_argument("--input", type=Path)
    a = ap.parse_args()
    raw = a.rom.read_bytes()
    for name, data, expected in [
        ("ROM", raw, EXPECTED_ROM_SHA256),
        ("PRG", raw[16:32784], EXPECTED_PRG_SHA256),
        ("CHR", raw[32784:], EXPECTED_CHR_SHA256),
    ]:
        actual = hashlib.sha256(data).hexdigest()
        if actual != expected:
            raise SystemExit(f"{name} SHA-256 mismatch; execution refused")
        print(f"{name.lower()}_sha256={actual}", flush=True)
    if a.instructions <= 0:
        raise SystemExit("instructions must be positive")
    a.frame.parent.mkdir(parents=True, exist_ok=True)
    cmd = [str(a.runner.resolve()), str(a.rom.resolve()), str(a.instructions), str(a.frame.resolve())]
    if a.input:
        cmd.append(str(a.input.resolve()))
    result = subprocess.run(cmd, check=False)
    if result.returncode == 0:
        print("frame_sha256=" + hashlib.sha256(a.frame.read_bytes()).hexdigest(), flush=True)
    raise SystemExit(result.returncode)

if __name__ == "__main__":
    main()
