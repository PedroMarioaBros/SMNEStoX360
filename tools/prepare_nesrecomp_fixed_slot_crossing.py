#!/usr/bin/env python3
"""Patch only a TEMPORARY pinned NESRecomp checkout for fixed-slot instruction tails.

NESRecomp's cycle backend ordinarily leaves an instruction for the interpreter
when any byte crosses a 4 KiB CPU slot. For fixed slots, every byte is still
known statically, so this experiment teaches discovery/emission to fetch from
the adjacent slot only when that slot's bank is fixed. Switchable slots and
instructions wrapping past $FFFF remain unsupported and fail closed.
"""
from __future__ import annotations

from pathlib import Path
import sys


def replace_once(path: Path, old: str, new: str) -> None:
    source = path.read_text(encoding="utf-8")
    count = source.count(old)
    if count != 1:
        raise SystemExit(
            f"ERROR: expected exactly one replacement in {path}, found {count}: "
            f"{old.splitlines()[0]!r}"
        )
    path.write_text(source.replace(old, new, 1), encoding="utf-8")


def main() -> int:
    if len(sys.argv) != 2:
        print(f"Usage: {Path(sys.argv[0]).name} TEMP_NESRECOMP_ROOT", file=sys.stderr)
        return 2
    root = Path(sys.argv[1]).resolve()
    codegen = root / "recompiler/src/cyc_codegen.c"
    if not codegen.is_file():
        raise SystemExit(f"ERROR: {root} is not the expected NESRecomp checkout")

    replace_once(
        codegen,
        """/* Whether every byte of the instruction is a byte this block knows: inside
 * $8000-$FFFF (the program counter wraps from $FFFF to $0000) and inside the
 * one slot whose bank the block was generated for. An instruction that
 * reaches into the next slot reads a byte from a bank chosen at run time, so
 * neither its operand nor its length can be folded; the interpreter runs it. */
static bool fits_in_slot(const Pos *at, int len) {
    uint16_t addr = pos_addr(at);
    if ((uint32_t)addr + (uint32_t)len - 1 > 0xFFFF) return false;
    return at->k + (uint32_t)len <= SLOT_SIZE;
}""",
        """/* An instruction can cross a 4 KiB slot only when every crossed slot is
 * permanently mapped to a known PRG bank. Wrapping past $FFFF or entering a
 * switchable slot is still unresolved and cannot be folded. */
static bool fits_in_slot(const Program *p, const Pos *at, int len) {
    uint16_t addr = pos_addr(at);
    if ((uint32_t)addr + (uint32_t)len - 1 > 0xFFFF) return false;
    for (int delta = 1; delta < len; delta++) {
        uint16_t byte_addr = (uint16_t)(addr + delta);
        uint32_t slot = (byte_addr >> SLOT_SHIFT) & (SLOT_COUNT - 1);
        if (slot != at->slot && p->fixed[slot] < 0) return false;
    }
    return true;
}""",
    )
    replace_once(
        codegen,
        """static bool target_pos(const Program *p, const Pos *from, uint16_t addr, Pos *out) {
    if (addr < 0x8000) return false;
    uint32_t slot = (addr >> SLOT_SHIFT) & (SLOT_COUNT - 1);
    uint32_t k = addr & (SLOT_SIZE - 1);
    if (slot == from->slot) {
        out->bank = from->bank;
    } else if (p->fixed[slot] >= 0) {
        out->bank = (uint32_t)p->fixed[slot];
    } else {
        return false;
    }
    out->slot = slot;
    out->k = k;
    return true;
}""",
        """static bool target_pos(const Program *p, const Pos *from, uint16_t addr, Pos *out) {
    if (addr < 0x8000) return false;
    uint32_t slot = (addr >> SLOT_SHIFT) & (SLOT_COUNT - 1);
    uint32_t k = addr & (SLOT_SIZE - 1);
    if (slot == from->slot) {
        out->bank = from->bank;
    } else if (p->fixed[slot] >= 0) {
        out->bank = (uint32_t)p->fixed[slot];
    } else {
        return false;
    }
    out->slot = slot;
    out->k = k;
    return true;
}

/* Fetch a byte belonging to one statically known instruction. Crossing out of
 * its own slot is safe only into a fixed slot, as checked by fits_in_slot(). */
static uint8_t insn_byte_at(const Program *p, const Pos *from, uint32_t delta) {
    uint16_t addr = (uint16_t)(pos_addr(from) + delta);
    Pos to;
    if (!target_pos(p, from, addr, &to)) return 0;
    return prg_byte(p, to.bank, to.k);
}""",
    )
    replace_once(codegen, "if (!fits_in_slot(&at, len)) continue;", "if (!fits_in_slot(p, &at, len)) continue;")
    for old, new in (
        ("prg_byte(p, at.bank, at.k + 1)", "insn_byte_at(p, &at, 1)"),
        ("prg_byte(p, at.bank, at.k + 2)", "insn_byte_at(p, &at, 2)"),
    ):
        # These direct reads are the relative/JMP/JSR instruction operands in
        # discovery; all must use the same bank-aware crossing rule.
        if codegen.read_text(encoding="utf-8").count(old) < 2:
            raise SystemExit(f"ERROR: expected discovery operand expression {old!r}")
        codegen.write_text(
            codegen.read_text(encoding="utf-8").replace(old, new),
            encoding="utf-8",
        )
    replace_once(
        codegen,
        "return prg_byte(e->p, e->at.bank, e->at.k + (uint32_t)k);",
        "return insn_byte_at(e->p, &e->at, (uint32_t)k);",
    )

    source = codegen.read_text(encoding="utf-8")
    if "static uint8_t insn_byte_at(" not in source or "fits_in_slot(p, &at, len)" not in source:
        raise SystemExit("ERROR: fixed-slot instruction-byte helpers were not installed")
    print("Applied temporary fixed-slot cross-boundary emission patch to pinned checkout.")
    print("Only permanently mapped adjacent slots are folded; switchable slots and $FFFF wrap remain unresolved.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
