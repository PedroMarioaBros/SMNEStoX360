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
        "void cyc_profile_dispatch(uint16_t pc, bool from_interpreter);\n",
    )
    replace_once(
        cpu_h,
        "static inline bool cpu_fetch_rom(uint16_t pc, uint8_t opcode) {\n"
        "    hw_cycle_start(pc, HW_FETCH);",
        "static inline bool cpu_fetch_rom(uint16_t pc, uint8_t opcode) {\n"
        "    if (pc == 0x8E04) cyc_profile_dispatch(pc, false);\n"
        "    hw_cycle_start(pc, HW_FETCH);",
    )
    replace_once(
        run_c,
        '#include "hw_internal.h"\n',
        '#include "hw_internal.h"\n#include "cpu6502.h"\n#include <stdio.h>\n',
    )
    hook = r'''
/* Evaluation-only profiler for SMB_v026's inline-argument tail dispatcher.
 * At $8E04, the JSR return address is on the 6502 stack and A selects a word
 * in the inline table. This works whether the dispatcher itself came from
 * compiled code or the interpreter. Emit each (callsite, selector) only once. */
typedef struct { uint16_t callsite; uint8_t selector; } CycProfileSeen;
#define CYC_PROFILE_MAX_DISPATCHES 512
static CycProfileSeen cyc_profile_seen[CYC_PROFILE_MAX_DISPATCHES];
static unsigned cyc_profile_seen_count;

void cyc_profile_dispatch(uint16_t pc, bool from_interpreter) {
    if (pc != 0x8E04) return;
    const uint8_t *ram = cyc_cpu_ram();
    uint8_t sp = cpu.s;
    uint16_t ret = (uint16_t)(ram[0x100u + ((sp + 1u) & 0xFFu)] |
                              ((uint16_t)ram[0x100u + ((sp + 2u) & 0xFFu)] << 8));
    uint16_t callsite = (uint16_t)(ret - 2u);
    uint8_t selector = cpu.a;
    for (unsigned i = 0; i < cyc_profile_seen_count; ++i) {
        if (cyc_profile_seen[i].callsite == callsite &&
            cyc_profile_seen[i].selector == selector) return;
    }
    if (cyc_profile_seen_count >= CYC_PROFILE_MAX_DISPATCHES) {
        fprintf(stderr, "[INLINE_DISPATCH_PROFILE_ERROR] observed callsite/selector cap exceeded\n");
        return;
    }
    cyc_profile_seen[cyc_profile_seen_count++] = (CycProfileSeen){callsite, selector};
    uint32_t entry = (uint32_t)ret + 1u + (uint32_t)selector * 2u;
    uint16_t target = 0xFFFFu;
    if (entry < 0xFFFFu && hw_prg_is_rom((uint16_t)entry) &&
        hw_prg_is_rom((uint16_t)(entry + 1u))) {
        uint8_t lo = hw_cart_prg_read((uint16_t)entry);
        uint8_t hi = hw_cart_prg_read((uint16_t)(entry + 1u));
        target = (uint16_t)(lo | ((uint16_t)hi << 8));
    }
    fprintf(stderr,
            "[INLINE_DISPATCH_PROFILE] source=%s callsite=$%04X selector=%u table_entry=$%04X decoded_target=$%04X\n",
            from_interpreter ? "interpreter" : "native", callsite, selector,
            entry <= 0xFFFFu ? (unsigned)entry : 0xFFFFu, target);
}

'''

'''

    replace_once(
        run_c,
        "/* One dispatch at an instruction boundary: recompiled ROM code, a compiled\n",
        hook + "/* One dispatch at an instruction boundary: recompiled ROM code, a compiled\n",
    )
    interp_file = cyc / "cpu6502_interp.c"
    replace_once(
        interp_file,
        "void cpu_interp_step(void) {\n    uint16_t pc = cpu.pc;",
        "void cpu_interp_step(void) {\n    if (cpu.pc == 0x8E04) cyc_profile_dispatch(cpu.pc, true);\n    uint16_t pc = cpu.pc;",
    )

    header = cpu_h.read_text(encoding="utf-8")
    source = run_c.read_text(encoding="utf-8")
    if "cyc_profile_dispatch(pc, false);" not in header:
        raise SystemExit("ERROR: profile hook is not installed in native opcode fetch")
    if "[INLINE_DISPATCH_PROFILE]" not in source or "decoded_target=$%04X" not in source:
        raise SystemExit("ERROR: dynamic dispatch target reporting is missing")
    if "cyc_profile_dispatch(cpu.pc, true);" not in interp_file.read_text(encoding="utf-8"):
        raise SystemExit("ERROR: profile hook was not installed in interpreter fetch")
    print("Applied temporary inline-dispatch profile hook to native fetch and interpreter steps.")
    print("Each observed $8E04 entry reports caller, selector, inline table word, and decoded target.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
