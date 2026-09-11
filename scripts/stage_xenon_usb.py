#!/usr/bin/env python3
"""Create a ROM-free XeLL USB staging directory from a verified Xenon ELF32."""
from __future__ import annotations
import hashlib
from pathlib import Path
import shutil
import subprocess
import sys


def main() -> int:
    if len(sys.argv) != 3:
        print(f"usage: {sys.argv[0]} <smb360.elf32> <output-dir>", file=sys.stderr)
        return 2
    elf = Path(sys.argv[1]).resolve()
    out = Path(sys.argv[2]).resolve()
    verifier = Path(__file__).with_name("verify_xenon_elf32.py")
    check = subprocess.run([sys.executable, str(verifier), str(elf)], text=True,
                           stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    if check.returncode != 0:
        sys.stderr.write(check.stderr)
        return check.returncode
    out.mkdir(parents=True, exist_ok=True)
    target = out / "xenon.elf"
    shutil.copyfile(elf, target)
    digest = hashlib.sha256(target.read_bytes()).hexdigest()
    (out / "SHA256SUMS.txt").write_text(f"{digest}  xenon.elf\n", encoding="ascii")
    (out / "README.txt").write_text(
        "SMB360 XeLL test package\n"
        "========================\n"
        "Copy xenon.elf to the root of a FAT32 USB device for XeLL Reloaded.\n"
        "Place the user-owned SMB ROM separately as smb.nes when the build requests it.\n"
        "The ROM is intentionally not included in this package.\n",
        encoding="utf-8")
    print(f"XeLL USB staging: PASS ({target}; sha256={digest})")
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
