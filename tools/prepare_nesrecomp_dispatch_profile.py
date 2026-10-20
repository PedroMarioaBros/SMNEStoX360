#!/usr/bin/env python3
"""Instrument a TEMPORARY NESRecomp runner to observe SMB_v026 inline dispatch.

The hook records targets actually selected at $8E04 while the interpreter-
enabled reference executes. It doesn't modify CPU registers, bus activity or
cycle counts. The upstream checkout must already contain the evaluation's
fixed-slot and cycle_inline_jmp source patches. No upstream source is vendored.
"""
from __future__ import annotations

import sys
from pathlib import Path


def replace_once(path: Path, old: str, new: str) -> None:
    source = path.read_text(encoding="utf-8")
    count = source.count(old)
    if count != 1:
        raise SystemExit(f"ERROR: expected one replacement in {path}, found {count}: {old.splitlines()[0]!r}")
    path.write_text(source.replace(old, new, 1), encoding="utf-8")


def main() -> int:
    if len(sys.argv) != 2:
        print(f"Usage: {Path(sys.argv[0]).name} TEMP_NESRECOMP_ROOT", file=sys.stderr)
        return 2
    root = Path(sys.argv[1]).resolve()
    cyc = root / "runner/cyc"
    cpu_h = cyc / "cpu6502.h"
    run_c = cyc / "cyc_run.c"
    if not cpu_h.is_file() or not run_c.is_file():
        raise SystemExit("ERROR: expected cpu6502.h and cyc_run.c in temporary NESRecomp checkout")

    replace_once(
        cpu_h,
        "extern Cpu6502 cpu;\n",
        "extern Cpu6502 cpu;\n"
        "/* Evaluation-only observed dynamic control-flow target recorder. */\n"
        "void cyc_profile_instruction(uint16_t pc, uint8_t opcode);\n",
    )
    replace_once(
        cpu_h,
        "static inline bool cpu_fetch_rom(uint16_t pc, uint8_t opcode) {\n"
        "    hw_cycle_start(pc, HW_FETCH);",
        "static inline bool cpu_fetch_rom(uint16_t pc, uint8_t opcode) {\n"
        "    cyc_profile_instruction(pc, opcode);\n"
        "    hw_cycle_start(pc, HW_FETCH);",
    )
    replace_once(
        run_c,
        '#include "hw_internal.h"\n',
        '#include "hw_internal.h"\n#include "cpu6502.h"\n#include <stdio.h>\n',
    )
    hook = r'''
/* Evaluation-only profiler for SMB_v026's inline-argument tail dispatcher.
 * 6502 JSR pushes its last operand address; the first inline target word is
 * therefore return_address + 1. A is the zero-based word selector. */
static bool cyc_profile_have_last;
static uint16_t cyc_profile_last_pc;
static uint8_t cyc_profile_last_opcode;

void cyc_profile_instruction(uint16_t pc, uint8_t opcode) {
    if (pc == 0x8E04 && cyc_profile_have_last && cyc_profile_last_opcode == 0x20) {
        uint16_t callsite = cyc_profile_last_pc;
        uint8_t selector = cpu.a;
        uint32_t entry = (uint32_t)callsite + 3u + (uint32_t)selector * 2u;
        uint16_t target = 0xFFFFu;
        if (entry < 0xFFFFu && hw_prg_is_rom((uint16_t)entry) &&
            hw_prg_is_rom((uint16_t)(entry + 1u))) {
            uint8_t lo = hw_cart_prg_read((uint16_t)entry);
            uint8_t hi = hw_cart_prg_read((uint16_t)(entry + 1u));
            target = (uint16_t)(lo | ((uint16_t)hi << 8));
        }
        fprintf(stderr,
                "[INLINE_DISPATCH_PROFILE] callsite=$%04X selector=%u table_entry=$%04X decoded_target=$%04X\n",
                callsite, selector, entry <= 0xFFFFu ? (unsigned)entry : 0xFFFFu, target);
    }
    cyc_profile_last_pc = pc;
    cyc_profile_last_opcode = opcode;
    cyc_profile_have_last = true;
}

'''
    replace_once(
        run_c,
        "/* One dispatch at an instruction boundary: recompiled ROM code, a compiled\n",
        hook + "/* One dispatch at an instruction boundary: recompiled ROM code, a compiled\n",
    )

    header = cpu_h.read_text(encoding="utf-8")
    source = run_c.read_text(encoding="utf-8")
    if "cyc_profile_instruction(pc, opcode);" not in header:
        raise SystemExit("ERROR: profile hook is not installed in native opcode fetch")
    if "[INLINE_DISPATCH_PROFILE]" not in source or "decoded_target=$%04X" not in source:
        raise SystemExit("ERROR: dynamic dispatch target reporting is missing")
    print("Applied temporary inline-dispatch profile hook to interpreter-enabled runner.")
    print("Each observed $8E04 entry reports caller, selector, inline table word, and decoded target.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
