#!/usr/bin/env python3
"""Verify the GitHub-owned assets and historical artifact without external services."""
import hashlib
import json
from pathlib import Path

def main():
    root = Path(__file__).resolve().parents[1]
    inventory = json.loads((root / "artifacts/inventory.json").read_text())
    for entry in inventory["files"]:
        path = root / entry["path"]
        raw = path.read_bytes()
        if len(raw) != entry["bytes"] or hashlib.sha256(raw).hexdigest() != entry["sha256"]:
            raise SystemExit(f"Integrity failure: {entry['path']}")
        print(f"OK {entry['path']}")
    rom = (root / "assets/canonical/SMB_v026.nes").read_bytes()
    if rom[16:32784] != (root / "assets/canonical/SMB_v026.prg").read_bytes():
        raise SystemExit("PRG differs from ROM")
    if rom[32784:] != (root / "assets/canonical/SMB_v026.chr").read_bytes():
        raise SystemExit("CHR differs from ROM")
    print("Repository assets and historical artifact: PASS")

if __name__ == "__main__":
    main()
