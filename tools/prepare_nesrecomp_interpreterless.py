#!/usr/bin/env python3
"""Apply an interpreter-free, fail-closed experiment to a TEMPORARY NESRecomp checkout.

The upstream repository is fetched at a pinned commit by evaluate_nesrecomp.sh.
This tool edits only that temporary checkout, not vendored upstream files here.
It removes cpu6502_interp.c and instruments native instruction flow so an
uncovered-PC trap reports its opcode, last observed control transfer, and a
short ring buffer of recently fetched native instructions. This is a diagnostic
harness, not a production patch.
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
    cyc_dir = root / "runner/cyc"
    cmake_file = cyc_dir / "cyc.cmake"
    run_file = cyc_dir / "cyc_run.c"
    cpu_header = cyc_dir / "cpu6502.h"
    if not all(p.is_file() for p in (cmake_file, run_file, cpu_header)):
        raise SystemExit(f"ERROR: {root} is not the expected NESRecomp checkout")

    # The interpreter translation unit owns a data-only opcode-length table
    # also used by analysis. Move just that table, not interpreter code, into
    # a separate utility translation unit.
    interp_file = cyc_dir / "cpu6502_interp.c"
    interp_source = interp_file.read_text(encoding="utf-8")
    table_marker = "/* Bytes each opcode takes from the instruction stream (BRK: 2). */"
    if interp_source.count(table_marker) != 1:
        raise SystemExit("ERROR: expected one cpu6502_op_length metadata table")
    table = interp_source.split(table_marker, 1)[1].strip()
    if not table.startswith("const uint8_t cpu6502_op_length[256] = {"):
        raise SystemExit("ERROR: interpreter opcode-length table format changed")
    table_file = cyc_dir / "cpu6502_op_length.c"
    if table_file.exists():
        raise SystemExit(f"ERROR: refusing to overwrite {table_file}")
    table_file.write_text('#include "cpu6502.h"' + chr(10) * 2 + table + chr(10), encoding="utf-8")

    replace_once(
        cmake_file,
        "    ${NESRECOMP_CYC_DIR}/cpu6502.c\n"
        "    ${NESRECOMP_CYC_DIR}/cpu6502_interp.c\n",
        "    ${NESRECOMP_CYC_DIR}/cpu6502.c\n"
        "    ${NESRECOMP_CYC_DIR}/cpu6502_op_length.c\n",
    )

    # Every statically compiled ROM instruction enters through cpu_fetch_rom.
    # Observe its PC/opcode. Once a native block returns, comparing its final
    # instruction's expected sequential PC with cpu.pc identifies the actual
    # source/target for a non-sequential transfer (branch, jump, return, IRQ/NMI).
    replace_once(
        cpu_header,
        "extern Cpu6502 cpu;\n",
        "extern Cpu6502 cpu;\n"
        "/* Evaluation-only native-flow instrumentation; defined by cyc_run.c. */\n"
        "void cyc_diag_instruction(uint16_t pc, uint8_t opcode);\n",
    )
    replace_once(
        cpu_header,
        "static inline bool cpu_fetch_rom(uint16_t pc, uint8_t opcode) {\n"
        "    hw_cycle_start(pc, HW_FETCH);",
        "static inline bool cpu_fetch_rom(uint16_t pc, uint8_t opcode) {\n"
        "    cyc_diag_instruction(pc, opcode);\n"
        "    hw_cycle_start(pc, HW_FETCH);",
    )

    replace_once(
        run_file,
        '#include "hw_internal.h"\n',
        '#include "hw_internal.h"\n#include "cpu6502.h"\n#include <stdio.h>\n#include <stdlib.h>\n',
    )
    diag_block = r'''
/* Evaluation-only ring buffer. Records native instruction starts without
 * altering the emulated bus, cycles, or registers. */
typedef struct { uint16_t pc; uint8_t opcode; } CycDiagInsn;
#define CYC_DIAG_HISTORY 24
static CycDiagInsn cyc_diag_history[CYC_DIAG_HISTORY];
static unsigned cyc_diag_history_next, cyc_diag_history_count;
static bool cyc_diag_have_last;
static uint16_t cyc_diag_last_pc;
static uint8_t cyc_diag_last_opcode;
static bool cyc_diag_have_transfer;
static uint16_t cyc_diag_transfer_pc, cyc_diag_transfer_target;
static uint8_t cyc_diag_transfer_opcode;

void cyc_diag_instruction(uint16_t pc, uint8_t opcode) {
    cyc_diag_history[cyc_diag_history_next] = (CycDiagInsn){pc, opcode};
    cyc_diag_history_next = (cyc_diag_history_next + 1) % CYC_DIAG_HISTORY;
    if (cyc_diag_history_count < CYC_DIAG_HISTORY) cyc_diag_history_count++;
    cyc_diag_last_pc = pc;
    cyc_diag_last_opcode = opcode;
    cyc_diag_have_last = true;
}

/* Called only after a native block yields at an instruction boundary. */
static void cyc_diag_native_exit(uint16_t next_pc) {
    if (!cyc_diag_have_last) return;
    uint16_t sequential = (uint16_t)(cyc_diag_last_pc + cpu6502_op_length[cyc_diag_last_opcode]);
    if (next_pc != sequential) {
        cyc_diag_transfer_pc = cyc_diag_last_pc;
        cyc_diag_transfer_opcode = cyc_diag_last_opcode;
        cyc_diag_transfer_target = next_pc;
        cyc_diag_have_transfer = true;
    }
}

static void cyc_diag_dump_miss(uint16_t pc) {
    unsigned bank = hw_prg_is_rom(pc) ? hw_prg_bank4(pc) : 0u;
    unsigned opcode = hw_prg_is_rom(pc) ? hw_cart_prg_read(pc) : 0x100u;
    fprintf(stderr, "[NATIVE_COVERAGE_MISS] pc=$%04X bank=%u opcode=", pc, bank);
    if (opcode <= 0xFFu) fprintf(stderr, "$%02X\n", opcode);
    else fprintf(stderr, "unmapped\n");
    if (cyc_diag_have_transfer) {
        fprintf(stderr,
                "[NATIVE_LAST_TRANSFER] from=$%04X opcode=$%02X to=$%04X\n",
                cyc_diag_transfer_pc, cyc_diag_transfer_opcode, cyc_diag_transfer_target);
    } else {
        fprintf(stderr, "[NATIVE_LAST_TRANSFER] no non-sequential transfer observed yet\n");
    }
    if (cyc_diag_have_last) {
        fprintf(stderr, "[NATIVE_LAST_INSN] pc=$%04X opcode=$%02X expected_next=$%04X actual_next=$%04X\n",
                cyc_diag_last_pc, cyc_diag_last_opcode,
                (uint16_t)(cyc_diag_last_pc + cpu6502_op_length[cyc_diag_last_opcode]), pc);
    }
    fprintf(stderr, "[NATIVE_INSN_HISTORY] newest-first count=%u\n", cyc_diag_history_count);
    for (unsigned n = 0; n < cyc_diag_history_count; n++) {
        unsigned i = (cyc_diag_history_next + CYC_DIAG_HISTORY - 1 - n) % CYC_DIAG_HISTORY;
        fprintf(stderr, "[NATIVE_INSN] pc=$%04X opcode=$%02X\n",
                cyc_diag_history[i].pc, cyc_diag_history[i].opcode);
    }
    fflush(stderr);
}
'''
    replace_once(
        run_file,
        "/* An instruction start from $6000 up that no compiled view covers:\n",
        diag_block + "\n/* An instruction start from $6000 up that no compiled view covers:\n",
    )
    replace_once(
        run_file,
        "            cyc_native_run();\n"
        "            cyc_run_native_cycles += cyc_cycle_count() - before;",
        "            cyc_native_run();\n"
        "            cyc_diag_native_exit(cpu.pc);\n"
        "            cyc_run_native_cycles += cyc_cycle_count() - before;",
    )
    replace_once(
        run_file,
        "            fprintf(stderr, \"[NATIVE_COVERAGE_MISS] pc=$%04X bank=%u\\n\",\n"
        "                    pc, hw_prg_is_rom(pc) ? hw_prg_bank4(pc) : 0u);\n"
        "            fflush(stderr);\n"
        "            exit(86);\n",
        "            cyc_diag_dump_miss(pc);\n"
        "            exit(86);\n",
    )

    cmake_source = cmake_file.read_text(encoding="utf-8")
    run_source = run_file.read_text(encoding="utf-8")
    header_source = cpu_header.read_text(encoding="utf-8")
    source_list = cmake_source.split("set(NESRECOMP_C_INCLUDE_DIRS", 1)[0]
    if "${NESRECOMP_CYC_DIR}/cpu6502_interp.c" in source_list:
        raise SystemExit("ERROR: interpreter source remains in the cycle runner source list")
    if "${NESRECOMP_CYC_DIR}/cpu6502_op_length.c" not in source_list:
        raise SystemExit("ERROR: data-only opcode-length metadata was not added")
    table_source = table_file.read_text(encoding="utf-8")
    if "cpu_interp_step" in table_source or "switch (opcode)" in table_source:
        raise SystemExit("ERROR: interpreter logic leaked into the metadata translation unit")
    if "cpu_interp_step();" in run_source:
        raise SystemExit("ERROR: a runtime call to cpu_interp_step remains")
    if "[NATIVE_COVERAGE_MISS]" not in run_source or "[NATIVE_LAST_TRANSFER]" not in run_source:
        raise SystemExit("ERROR: fail-closed coverage/flow diagnostics missing")
    if "cyc_diag_instruction(pc, opcode);" not in header_source:
        raise SystemExit("ERROR: native instruction observer was not installed")

    print("Applied temporary no-interpreter experiment to pinned checkout.")
    print("Interpreter translation unit removed; native-flow ring buffer installed.")
    print("Uncovered dispatch exits with PC/bank/opcode, last transfer origin/target, and recent instruction history.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
