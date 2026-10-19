#!/usr/bin/env python3
"""Apply an interpreter-free, fail-closed experiment to a TEMPORARY NESRecomp checkout.

The upstream repository is fetched at a pinned commit by evaluate_nesrecomp.sh.
This tool edits only that temporary checkout, not vendored upstream files here.
It removes cpu6502_interp.c from the cycle runner source list and replaces its
only runtime call site with a coverage-miss trap. It is an evaluation harness,
not a production patch.
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
    cmake_file = root / "runner/cyc/cyc.cmake"
    run_file = root / "runner/cyc/cyc_run.c"
    if not cmake_file.is_file() or not run_file.is_file():
        raise SystemExit(f"ERROR: {root} is not the expected NESRecomp checkout")

    # The interpreter translation unit also happens to own an opcode-length
    # metadata table used by RAM-view analysis. Move that data-only table into
    # a separate utility translation unit rather than retaining any interpreter
    # function code in the executable.
    interp_file = root / "runner/cyc/cpu6502_interp.c"
    interp_source = interp_file.read_text(encoding="utf-8")
    table_marker = "/* Bytes each opcode takes from the instruction stream (BRK: 2). */"
    if interp_source.count(table_marker) != 1:
        raise SystemExit("ERROR: expected one cpu6502_op_length metadata table")
    table = interp_source.split(table_marker, 1)[1].strip()
    if not table.startswith("const uint8_t cpu6502_op_length[256] = {"):
        raise SystemExit("ERROR: interpreter opcode-length table format changed")
    table_file = root / "runner/cyc/cpu6502_op_length.c"
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
    replace_once(
        run_file,
        '#include "hw_internal.h"\n',
        '#include "hw_internal.h"\n#include <stdio.h>\n#include <stdlib.h>\n',
    )
    replace_once(
        run_file,
        "            cpu_interp_step();\n",
        "            fprintf(stderr, \"[NATIVE_COVERAGE_MISS] pc=$%04X bank=%u\\n\",\n"
        "                    pc, hw_prg_is_rom(pc) ? hw_prg_bank4(pc) : 0u);\n"
        "            fflush(stderr);\n"
        "            exit(86);\n",
    )

    cmake_source = cmake_file.read_text(encoding="utf-8")
    run_source = run_file.read_text(encoding="utf-8")
    source_list = cmake_source.split("set(NESRECOMP_C_INCLUDE_DIRS", 1)[0]
    if "${NESRECOMP_CYC_DIR}/cpu6502_interp.c" in source_list:
        raise SystemExit("ERROR: interpreter source remains in the cycle runner source list")
    if "${NESRECOMP_CYC_DIR}/cpu6502_op_length.c" not in source_list:
        raise SystemExit("ERROR: data-only opcode-length metadata was not added")
    table_source = (root / "runner/cyc/cpu6502_op_length.c").read_text(encoding="utf-8")
    if "cpu_interp_step" in table_source or "switch (opcode)" in table_source:
        raise SystemExit("ERROR: interpreter logic leaked into the metadata translation unit")
    if "cpu_interp_step();" in run_source:
        raise SystemExit("ERROR: a runtime call to cpu_interp_step remains")
    if "[NATIVE_COVERAGE_MISS]" not in run_source:
        raise SystemExit("ERROR: fail-closed native coverage trap missing")

    print("Applied temporary no-interpreter experiment to pinned checkout.")
    print("Interpreter translation unit removed from the build source list.")
    print("Any uncovered dispatch terminates with exit code 86 and its PC.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
