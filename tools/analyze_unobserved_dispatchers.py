#!/usr/bin/env python3
"""Verify statically identified parent-dispatch paths for six SMB_v026 callsites.

This is a ROM-specific evidence checker, not a 6502 emulator or a proof of
runtime reachability. It validates exact bytes and inline table words against
the pinned canonical ROM, then reports selector/condition hypotheses that need
dynamic confirmation. It never edits the ROM or a compiler configuration.
"""
from __future__ import annotations

import hashlib
import sys
from pathlib import Path

EXPECTED_SHA256 = "57fb4ee14288853bc0c8a5a629a103e51e87a4ebae1cc699b435060d2690b047"
EXPECTED_SIZE = 40976
DISPATCH = b"\x20\x04\x8E"


def addr(value: int) -> str:
    return "$%04X" % value


def main() -> int:
    if len(sys.argv) != 2:
        print(f"Usage: {Path(sys.argv[0]).name} CANONICAL_ROM", file=sys.stderr)
        return 2

    path = Path(sys.argv[1])
    if not path.is_file():
        raise SystemExit(f"ERROR: canonical ROM does not exist: {path}")
    rom = path.read_bytes()
    digest = hashlib.sha256(rom).hexdigest()
    if len(rom) != EXPECTED_SIZE or digest != EXPECTED_SHA256:
        raise SystemExit(
            f"ERROR: canonical ROM identity mismatch: size={len(rom)} sha256={digest}"
        )
    if rom[:4] != b"NES\x1a" or rom[4] != 2 or rom[5] != 1:
        raise SystemExit("ERROR: expected canonical iNES ROM with 32 KiB PRG and 8 KiB CHR")
    if ((rom[6] >> 4) | (rom[7] & 0xF0)) != 0 or (rom[6] & 0x04):
        raise SystemExit("ERROR: expected mapper 0 and no iNES trainer")

    prg = rom[16:16 + 32768]

    def read(address: int, size: int) -> bytes:
        if not 0x8000 <= address <= 0xFFFF or address + size > 0x10000:
            raise ValueError(f"address outside linear NROM PRG: {addr(address)} size={size}")
        offset = address - 0x8000
        return prg[offset:offset + size]

    def check_bytes(address: int, expected: bytes, label: str) -> None:
        actual = read(address, len(expected))
        if actual != expected:
            actual_hex = " ".join(f"{b:02X}" for b in actual)
            expected_hex = " ".join(f"{b:02X}" for b in expected)
            raise SystemExit(
                f"ERROR: {label} at {addr(address)}: expected [{expected_hex}], got [{actual_hex}]"
            )

    def check_dispatch(callsite: int, selector: int, target: int) -> int:
        check_bytes(callsite, DISPATCH, f"dispatch callsite {addr(callsite)}")
        entry = callsite + 3 + selector * 2
        actual = int.from_bytes(read(entry, 2), "little")
        if actual != target:
            raise SystemExit(
                f"ERROR: {addr(callsite)} selector {selector}: expected target {addr(target)} "
                f"at {addr(entry)}, got {addr(actual)}"
            )
        return entry

    for address in (0x83A3, 0x921B, 0x9743, 0xC7A8, 0xC987, 0xD2E5):
        check_bytes(address, DISPATCH, f"pending dispatcher {addr(address)}")
    for address in (0x8215, 0x9664, 0xC27F, 0xC88F):
        check_bytes(address, DISPATCH, f"parent dispatcher {addr(address)}")

    rows: list[tuple[int, str, list[str], str]] = []

    entry = check_dispatch(0x8215, 2, 0x838B)
    check_bytes(0x838B, b"\x20\xA0\x83", "JSR $83A0")
    check_bytes(0x83A0, b"\xAD\x72\x07", "block preceding $83A3")
    rows.append((0x83A3, "$8215 selector 2 -> $838B -> JSR $83A0 -> $83A3",
                 [f"table_entry={addr(entry)}"], "selector 2 not seen in prior four scenarios"))

    entry = check_dispatch(0x8215, 3, 0x9218)
    check_bytes(0x9218, b"\xAD\x72\x07", "block preceding $921B")
    rows.append((0x921B, "$8215 selector 3 -> $9218 -> $921B",
                 [f"table_entry={addr(entry)}"], "selector 3 not seen in prior four scenarios"))

    entry = check_dispatch(0x9664, 1, 0x9740)
    check_bytes(0x9740, b"\xAD\x33\x07", "block preceding $9743")
    rows.append((0x9743, "$9664 selector 1 -> $9740 -> $9743",
                 [f"table_entry={addr(entry)}"], "selector 1 not seen in prior four scenarios"))

    c7_entries = []
    for selector in (18, 20, 21, 22, 23):
        c7_entries.append(check_dispatch(0xC27F, selector, 0xC7A0))
    check_bytes(0xC7A0, b"\xB5\x16\x8D\xCB\x06\x38\xE9\x12", "block preceding $C7A8")
    rows.append((0xC7A8, "$C27F selectors 18,20,21,22,23 -> $C7A0 -> $C7A8",
                 [f"table_entries={','.join(addr(x) for x in c7_entries)}"],
                 "none of these selectors seen in prior four scenarios"))

    c9_entries = []
    for selector in range(16, 23):
        c9_entries.append(check_dispatch(0xC88F, selector, 0xC965))
    check_bytes(0xC971, b"\xAD\x47\x07\xD0\x03", "conditional read/branch before $C987")
    check_bytes(0xC976, b"\x20\x82\xC9", "conditional JSR $C982")
    check_bytes(0xC982, b"\xB5\x16\x38\xE9\x24", "block preceding $C987")
    rows.append((0xC987, "$C88F selectors 16..22 -> $C965; if $0747 == 0, JSR $C982 -> $C987",
                 [f"table_entries={','.join(addr(x) for x in c9_entries)}"],
                 "selectors/condition not confirmed dynamically"))

    entry = check_dispatch(0xC88F, 29, 0xD2D9)
    check_bytes(0xD2D9, b"\xA9\x00\x8D\xCB\x06\xAD\x46\x07\xC9\x05\xB0\x2C",
                "block and conditional branch before $D2E5")
    check_bytes(0xD2E5, DISPATCH, "target dispatcher $D2E5")
    branch_target = 0xD2E5 + 0x2C
    if branch_target != 0xD311:
        raise SystemExit(f"ERROR: expected BCS target $D311, computed {addr(branch_target)}")
    rows.append((0xD2E5, "$C88F selector 29 -> $D2D9; reaches $D2E5 only when $0746 < 5",
                 [f"table_entry={addr(entry)}", "BCS skip target=$D311"],
                 "selector/condition not confirmed dynamically"))

    print(f"[DISPATCH_PATH_VERIFY] sha256={digest} prg_bytes=32768 mapper=0")
    print("[DISPATCH_PATH_VERIFY] parent table words and intermediate ROM bytes: PASS")
    for address, route, extra, status in rows:
        print(f"[DISPATCH_PATH] callsite={addr(address)} route=\"{route}\" "
              f"{' '.join(extra)} dynamic_status=\"{status}\"")
    print("[DISPATCH_PATH_VERIFY] PASS — static edges validated; not a dynamic-coverage claim.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
