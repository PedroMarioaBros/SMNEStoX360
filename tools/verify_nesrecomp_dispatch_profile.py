#!/usr/bin/env python3
"""Validate profile-observed SMB_v026 inline dispatches against the canonical ROM.

This is a verification/reporting tool, not a general 6502 disassembler. It
checks that each recorded callsite is an actual JSR $8E04 in the exact NROM-256
PRG, and that each selected table word decodes to the profiler's target.
"""
from __future__ import annotations

import hashlib
import re
import sys
from pathlib import Path

EXPECTED_SHA256 = "57fb4ee14288853bc0c8a5a629a103e51e87a4ebae1cc699b435060d2690b047"
EXPECTED_SIZE = 40976
PROFILE_LINE = re.compile(
    r"^\[INLINE_DISPATCH_PROFILE\] (?:source=(?:native|interpreter) )?"
    r"callsite=\$([0-9A-Fa-f]{4}) selector=(\d+) "
    r"table_entry=\$([0-9A-Fa-f]{4}) decoded_target=\$([0-9A-Fa-f]{4})$"
)


def main() -> int:
    if len(sys.argv) != 3:
        print(f"Usage: {Path(sys.argv[0]).name} CANONICAL_ROM DISPATCH_PROFILE_LOG", file=sys.stderr)
        return 2
    rom_path, profile_path = map(Path, sys.argv[1:])
    if not rom_path.is_file() or not profile_path.is_file():
        raise SystemExit("ERROR: canonical ROM or dispatch profile log does not exist")
    rom = rom_path.read_bytes()
    digest = hashlib.sha256(rom).hexdigest()
    if len(rom) != EXPECTED_SIZE or digest != EXPECTED_SHA256:
        raise SystemExit(f"ERROR: canonical ROM identity mismatch: size={len(rom)} sha256={digest}")
    if rom[:4] != b"NES\x1a":
        raise SystemExit("ERROR: invalid iNES signature")
    if rom[6] & 0x04:
        raise SystemExit("ERROR: unexpected iNES trainer; verifier currently expects the exact canonical layout")
    mapper = (rom[6] >> 4) | (rom[7] & 0xF0)
    prg_size = rom[4] * 16384
    if mapper != 0 or prg_size != 32768:
        raise SystemExit(f"ERROR: expected mapper 0 / 32 KiB PRG, got mapper={mapper} PRG={prg_size}")

    prg = rom[16:16 + prg_size]
    # CPU $8000-$FFFF maps linearly to the PRG image for this NROM-256 ROM.
    def read_prg_word(addr: int) -> int:
        if not 0x8000 <= addr < 0xFFFF:
            raise ValueError(f"not a two-byte NROM PRG address: 0x{addr:04X}")
        off = addr - 0x8000
        return prg[off] | (prg[off + 1] << 8)

    raw_callsites = {
        0x8000 + off
        for off in range(len(prg) - 2)
        if prg[off:off + 3] == b"\x20\x04\x8E"
    }

    observed: dict[tuple[int, int, int, int], set[str]] = {}
    malformed = 0
    for raw in profile_path.read_text(encoding="utf-8", errors="replace").splitlines():
        if "[INLINE_DISPATCH_PROFILE]" not in raw:
            continue
        line = raw[raw.index("[INLINE_DISPATCH_PROFILE]"):].strip()
        m = PROFILE_LINE.match(line)
        if not m:
            malformed += 1
            continue
        callsite, selector, table_entry, target = int(m.group(1), 16), int(m.group(2)), int(m.group(3), 16), int(m.group(4), 16)
        expected_table = callsite + 3 + 2 * selector
        if expected_table > 0xFFFF or table_entry != expected_table:
            raise SystemExit(
                f"ERROR: table-entry arithmetic mismatch at callsite ${callsite:04X} selector={selector}: "
                f"logged ${table_entry:04X}, expected ${expected_table:05X}"
            )
        if callsite not in raw_callsites:
            raise SystemExit(f"ERROR: observed profile callsite ${callsite:04X} is not a JSR $8E04 in canonical PRG")
        if prg[callsite - 0x8000:callsite - 0x8000 + 3] != b"\x20\x04\x8E":
            raise SystemExit(f"ERROR: canonical bytes do not encode JSR $8E04 at ${callsite:04X}")
        actual_target = read_prg_word(table_entry)
        if actual_target != target:
            raise SystemExit(
                f"ERROR: profile target mismatch at ${callsite:04X} selector={selector}: "
                f"ROM word=${actual_target:04X}, profiler=${target:04X}"
            )
        source_m = re.search(r"source=(native|interpreter)", line)
        source = source_m.group(1) if source_m else "unspecified"
        observed.setdefault((callsite, selector, table_entry, target), set()).add(source)

    if malformed:
        raise SystemExit(f"ERROR: {malformed} malformed dispatch profile records")
    if not observed:
        raise SystemExit("ERROR: no validated inline-dispatch entries found")
    callsites = {key[0] for key in observed}
    targets = {key[3] for key in observed if 0x8000 <= key[3] <= 0xFFFE}
    source_counts = {
        "native": sum("native" in v for v in observed.values()),
        "interpreter": sum("interpreter" in v for v in observed.values()),
    }
    selectors_by_callsite: dict[int, set[int]] = {}
    for callsite, selector, _entry, _target in observed:
        selectors_by_callsite.setdefault(callsite, set()).add(selector)
    unobserved = sorted(raw_callsites - callsites)
    print(f"[DISPATCH_ROM_VERIFY] sha256={digest} mapper={mapper} prg_bytes={prg_size}")
    print(f"[DISPATCH_ROM_VERIFY] raw_jsr_8e04_candidates={len(raw_callsites)} observed_callsites={len(callsites)}")
    print(f"[DISPATCH_ROM_VERIFY] unique_callsite_selector_entries={len(observed)} unique_ROM_targets={len(targets)}")
    print(f"[DISPATCH_ROM_VERIFY] observed_sources native={source_counts['native']} interpreter={source_counts['interpreter']}")
    print("[DISPATCH_ROM_VERIFY] callsite bytes and selected table words: PASS")
    print(f"[DISPATCH_ROM_VERIFY] unobserved_raw_candidates={len(unobserved)}")
    for callsite in sorted(raw_callsites):
        selectors = selectors_by_callsite.get(callsite, set())
        status = "observed" if selectors else "not-observed"
        selector_text = ",".join(str(value) for value in sorted(selectors)) or "none"
        off = callsite - 0x8000
        context = prg[off:off + 7].hex(" ").upper()
        print(f"[DISPATCH_ROM_CALLSITE] addr=${callsite:04X} status={status} observed_selectors={selector_text} bytes={context}")
    for callsite, selector, entry, target in sorted(observed):
        source_list = ",".join(sorted(observed[(callsite, selector, entry, target)]))
        print(f"[DISPATCH_ROM_ENTRY] caller=${callsite:04X} selector={selector} entry=${entry:04X} target=${target:04X} source={source_list}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
