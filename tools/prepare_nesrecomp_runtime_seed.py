#!/usr/bin/env python3
"""Add one observed ROM target to a TEMPORARY evaluation config.

This is a bounded, coverage-guided experiment: the no-interpreter runner traps
at an uncovered PC, and the harness can add that reachable target as a static
function seed before regenerating. The canonical ROM and repository config are
never modified; only a temporary copy of the TOML is edited.
"""
from __future__ import annotations

import re
import sys
from pathlib import Path


def main() -> int:
    if len(sys.argv) != 3:
        print(f"Usage: {Path(sys.argv[0]).name} TEMP_CONFIG.toml PC", file=sys.stderr)
        return 2
    path = Path(sys.argv[1])
    if not path.is_file():
        raise SystemExit(f"ERROR: temporary config not found: {path}")
    raw_pc = sys.argv[2].strip().replace("$", "0x")
    try:
        pc = int(raw_pc, 0)
    except ValueError as exc:
        raise SystemExit(f"ERROR: invalid PC: {sys.argv[2]!r}") from exc
    if not 0x8000 <= pc <= 0xFFFF:
        raise SystemExit(f"ERROR: refusing to seed non-ROM address ${pc:04X}")

    source = path.read_text(encoding="utf-8")
    match = re.search(r"(?ms)(^\[functions\]\s*\n\s*fixed\s*=\s*\[)(.*?)(\n\s*\])", source)
    if not match:
        raise SystemExit("ERROR: could not locate [functions].fixed array in temporary config")
    body = match.group(2)
    values = re.findall(r"(?i)(?<![A-Za-z0-9_])(?:0x[0-9a-f]+|\b[0-9]+\b)", body)
    known = {int(value, 0) for value in values}
    if pc in known:
        raise SystemExit(f"ERROR: coverage miss ${pc:04X} is already seeded; stopping to avoid an infinite retry")
    insert = body.rstrip()
    if insert and not insert.endswith(","):
        insert += ","
    insert += f"\n  0x{pc:04X}"
    updated = source[:match.start(2)] + insert + source[match.end(2):]
    path.write_text(updated, encoding="utf-8")
    print(f"[AUTO_NATIVE_SEED] added observed ROM target 0x{pc:04X} to the temporary config")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
