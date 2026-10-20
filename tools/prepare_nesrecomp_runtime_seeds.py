#!/usr/bin/env python3
"""Collect interpreter-miss PCs from a deterministic reference profile.

The resulting target seeds are appended only to a TEMPORARY game TOML. This is
profile-guided static recompilation (not a proof of whole-ROM reachability),
and the runtime used for the final native-only test is rebuilt with the
interpreter translation unit removed and a fail-closed miss trap installed.
"""
from __future__ import annotations

import re
import sys
from pathlib import Path


MAX_PROFILE_SEEDS = 256


def main() -> int:
    if len(sys.argv) != 3:
        print(f"Usage: {Path(sys.argv[0]).name} TEMP_CONFIG.toml REFERENCE_MISS_LOG", file=sys.stderr)
        return 2
    config_path, log_path = map(Path, sys.argv[1:])
    if not config_path.is_file() or not log_path.is_file():
        raise SystemExit("ERROR: temporary config or reference miss-log does not exist")

    # The reference runner writes '4k:BB:AAAA count'. Also accept the simpler
    # NROM 'AAAA count' form. Mapper 0 uses fixed PRG, so CPU address is enough
    # for the temporary [functions].fixed seed list.
    counts: dict[int, int] = {}
    banks: dict[int, set[int]] = {}
    for raw in log_path.read_text(encoding="utf-8", errors="replace").splitlines():
        line = raw.strip()
        if not line or line.startswith("#"):
            continue
        m = re.match(r"(?i)^4k:([0-9a-f]{1,4}):([0-9a-f]{4})\s+(\d+)", line)
        if m:
            bank = int(m.group(1), 16)
            pc = int(m.group(2), 16)
            count = int(m.group(3))
        else:
            m = re.match(r"(?i)^([0-9a-f]{4})\s+(\d+)", line)
            if not m:
                continue
            pc = int(m.group(1), 16)
            count = int(m.group(2))
            bank = -1
        if not 0x8000 <= pc <= 0xFFFF:
            continue
        counts[pc] = counts.get(pc, 0) + count
        banks.setdefault(pc, set()).add(bank)

    if not counts:
        raise SystemExit("ERROR: reference profile reported no interpreted ROM instruction starts")

    source = config_path.read_text(encoding="utf-8")
    match = re.search(r"(?ms)(^\[functions\]\s*\n\s*fixed\s*=\s*\[)(.*?)(\n\s*\])", source)
    if not match:
        raise SystemExit("ERROR: could not locate [functions].fixed array in temporary config")
    body = match.group(2)
    values = re.findall(r"(?i)(?<![A-Za-z0-9_])(?:0x[0-9a-f]+|\b[0-9]+\b)", body)
    existing = {int(value, 0) for value in values}
    targets = sorted(pc for pc in counts if pc not in existing)
    if len(targets) > MAX_PROFILE_SEEDS:
        raise SystemExit(f"ERROR: profile discovered {len(targets)} new ROM seeds; limit is {MAX_PROFILE_SEEDS}")
    if not targets:
        raise SystemExit("ERROR: profile miss-log has addresses, but none are new to the temporary config")

    updated_body = body.rstrip()
    if updated_body and not updated_body.endswith(","):
        updated_body += ","
    for pc in targets:
        updated_body += f"\n  0x{pc:04X},"
    updated_body = updated_body.rstrip(",")
    config_path.write_text(source[:match.start(2)] + updated_body + source[match.end(2):], encoding="utf-8")

    for pc in targets:
        bank_text = ",".join(f"{b:02X}" for b in sorted(banks[pc]) if b >= 0) or "NROM-fixed"
        print(f"[PROFILE_NATIVE_SEED] pc=${pc:04X} bank={bank_text} interpreter_starts={counts[pc]}")
    print(f"[PROFILE_NATIVE_SEED_SUMMARY] reference_unique_ROM_misses={len(counts)} new_static_seeds={len(targets)}")
    print("All profile seeds were written only to the temporary config; repository TOML is unchanged.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
