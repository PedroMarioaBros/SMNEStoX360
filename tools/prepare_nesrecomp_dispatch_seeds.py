#!/usr/bin/env python3
"""Add observed inline-dispatch destinations to a TEMPORARY NESRecomp config.

This is profile-guided recompilation for the exact canonical ROM: only targets
actually selected at the instrumented $8E04 dispatcher in deterministic
reference runs are added. It is not a proof of exhaustive ROM reachability.
"""
from __future__ import annotations

import re
import sys
from collections import Counter
from pathlib import Path


MAX_PROFILE_TARGETS = 256
LINE = re.compile(
    r"^\[INLINE_DISPATCH_PROFILE\] (?:source=(?:native|interpreter) )?callsite=\$([0-9A-Fa-f]{4}) "
    r"selector=(\d+) table_entry=\$([0-9A-Fa-f]{4}) "
    r"decoded_target=\$([0-9A-Fa-f]{4})$"
)


def main() -> int:
    if len(sys.argv) != 3:
        print(f"Usage: {Path(sys.argv[0]).name} TEMP_CONFIG.toml DISPATCH_PROFILE.log", file=sys.stderr)
        return 2
    config_path, log_path = map(Path, sys.argv[1:])
    if not config_path.is_file() or not log_path.is_file():
        raise SystemExit("ERROR: temporary config or dispatch profile log does not exist")

    observed: Counter[int] = Counter()
    sites: set[tuple[int, int, int]] = set()
    malformed = 0
    for raw in log_path.read_text(encoding="utf-8", errors="replace").splitlines():
        if "[INLINE_DISPATCH_PROFILE]" not in raw:
            continue
        line = raw[raw.index("[INLINE_DISPATCH_PROFILE]"):].strip()
        m = LINE.match(line)
        if not m:
            malformed += 1
            continue
        callsite = int(m.group(1), 16)
        selector = int(m.group(2), 10)
        table_entry = int(m.group(3), 16)
        target = int(m.group(4), 16)
        expected_entry = callsite + 3 + selector * 2
        if expected_entry > 0xFFFF or table_entry != expected_entry:
            raise SystemExit(
                f"ERROR: inconsistent inline entry math at callsite ${callsite:04X}, "
                f"selector {selector}: logged ${table_entry:04X}, expected ${expected_entry:05X}"
            )
        if not 0x8000 <= target <= 0xFFFE:
            continue
        observed[target] += 1
        sites.add((callsite, selector, target))

    if malformed:
        raise SystemExit(f"ERROR: {malformed} malformed inline-dispatch profile records")
    if not observed:
        raise SystemExit("ERROR: no ROM targets observed at the instrumented inline dispatcher")
    if len(observed) > MAX_PROFILE_TARGETS:
        raise SystemExit(f"ERROR: profile discovered {len(observed)} unique ROM targets; limit is {MAX_PROFILE_TARGETS}")

    source = config_path.read_text(encoding="utf-8")
    match = re.search(r"(?ms)(^\[functions\]\s*\n\s*fixed\s*=\s*\[)(.*?)(\n\s*\])", source)
    if not match:
        raise SystemExit("ERROR: could not locate [functions].fixed array in temporary config")
    body = match.group(2)
    values = re.findall(r"(?i)(?<![A-Za-z0-9_])(?:0x[0-9a-f]+|\b[0-9]+\b)", body)
    existing = {int(value, 0) for value in values}
    targets = sorted(set(observed) - existing)
    if not targets:
        raise SystemExit("ERROR: all observed dispatcher targets are already configured; no new profile targets to apply")

    new_body = body.rstrip()
    if new_body and not new_body.endswith(","):
        new_body += ","
    for target in targets:
        new_body += f"\n  0x{target:04X},"
    new_body = new_body.rstrip(",")
    config_path.write_text(source[:match.start(2)] + new_body + source[match.end(2):], encoding="utf-8")

    print(f"[DISPATCH_PROFILE_SUMMARY] observed_dispatch_records={len(sites)} unique_selected_targets={len(observed)}")
    print(f"[DISPATCH_PROFILE_SUMMARY] new_temporary_static_seeds={len(targets)} malformed_records={malformed}")
    for target in targets:
        print(f"[DISPATCH_PROFILE_SEED] target=${target:04X} observations={observed[target]}")
    print("All selected targets were appended only to the temporary config; canonical ROM and repository TOML were unchanged.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
