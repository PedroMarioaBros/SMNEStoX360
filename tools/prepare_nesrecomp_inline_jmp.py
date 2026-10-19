#!/usr/bin/env python3
"""Add a configurable inline-tail-jump annotation to a TEMPORARY NESRecomp checkout.

Some 6502 code calls an inline dispatcher which consumes the two bytes following
JSR as a destination, then transfers there via RTS-style stack mechanics. The
ordinary cycle_inline_jsr annotation is for routines that return past inline
arguments; it incorrectly treats the data word after a tail dispatcher as
fall-through code. This experiment adds cycle_inline_jmp = ["ADDR:BYTES"] which
suppresses only that false fall-through while still analysing the dispatcher
itself. Changes are applied only to the pinned upstream checkout in CI.
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
    cfg_h = root / "recompiler/src/game_config.h"
    cfg_c = root / "recompiler/src/game_config.c"
    cyc = root / "recompiler/src/cyc_codegen.c"
    for path in (cfg_h, cfg_c, cyc):
        if not path.is_file():
            raise SystemExit(f"ERROR: missing expected source: {path}")

    replace_once(
        cfg_h,
        """    struct { uint16_t target; uint8_t bytes; } cycle_inline_jsr[64];
    int              cycle_inline_jsr_count;""",
        """    struct { uint16_t target; uint8_t bytes; } cycle_inline_jsr[64];
    int              cycle_inline_jsr_count;
    /* [game] cycle_inline_jmp = ["8E04:2", ...]: inline tail dispatchers.
     * The word after JSR supplies a destination; do not analyse it as fall-through. */
    struct { uint16_t target; uint8_t bytes; } cycle_inline_jmp[64];
    int              cycle_inline_jmp_count;""",
    )

    replace_once(
        cfg_c,
        """        toml_array_t *inl = toml_array_in(game, "cycle_inline_jsr");
        for (int i = 0; inl && i < toml_array_nelem(inl) && cfg->cycle_inline_jsr_count < 64; i++) {
            toml_datum_t d = toml_string_at(inl, i);
            if (!d.ok) continue;
            unsigned target, bytes;
            if (sscanf(d.u.s, "%x:%u", &target, &bytes) == 2 && target <= 0xFFFF && bytes <= 255) {
                cfg->cycle_inline_jsr[cfg->cycle_inline_jsr_count].target = (uint16_t)target;
                cfg->cycle_inline_jsr[cfg->cycle_inline_jsr_count].bytes = (uint8_t)bytes;
                cfg->cycle_inline_jsr_count++;
            } else {
                fprintf(stderr, "[GameConfig] Warning: cycle_inline_jsr entry '%s' is not ADDR:BYTES\\n", d.u.s);
            }
            free(d.u.s);
        }""",
        """        toml_array_t *inl = toml_array_in(game, "cycle_inline_jsr");
        for (int i = 0; inl && i < toml_array_nelem(inl) && cfg->cycle_inline_jsr_count < 64; i++) {
            toml_datum_t d = toml_string_at(inl, i);
            if (!d.ok) continue;
            unsigned target, bytes;
            if (sscanf(d.u.s, "%x:%u", &target, &bytes) == 2 && target <= 0xFFFF && bytes <= 255) {
                cfg->cycle_inline_jsr[cfg->cycle_inline_jsr_count].target = (uint16_t)target;
                cfg->cycle_inline_jsr[cfg->cycle_inline_jsr_count].bytes = (uint8_t)bytes;
                cfg->cycle_inline_jsr_count++;
            } else {
                fprintf(stderr, "[GameConfig] Warning: cycle_inline_jsr entry '%s' is not ADDR:BYTES\\n", d.u.s);
            }
            free(d.u.s);
        }
        toml_array_t *inlj = toml_array_in(game, "cycle_inline_jmp");
        for (int i = 0; inlj && i < toml_array_nelem(inlj) && cfg->cycle_inline_jmp_count < 64; i++) {
            toml_datum_t d = toml_string_at(inlj, i);
            if (!d.ok) continue;
            unsigned target, bytes;
            if (sscanf(d.u.s, "%x:%u", &target, &bytes) == 2 && target <= 0xFFFF && bytes <= 255) {
                cfg->cycle_inline_jmp[cfg->cycle_inline_jmp_count].target = (uint16_t)target;
                cfg->cycle_inline_jmp[cfg->cycle_inline_jmp_count].bytes = (uint8_t)bytes;
                cfg->cycle_inline_jmp_count++;
            } else {
                fprintf(stderr, "[GameConfig] Warning: cycle_inline_jmp entry '%s' is not ADDR:BYTES\\n", d.u.s);
            }
            free(d.u.s);
        }""",
    )

    replace_once(
        cyc,
        "    uint8_t  inline_jsr[0x10000];",
        "    uint8_t  inline_jsr[0x10000];\n    uint8_t  inline_jmp[0x10000];",
    )
    replace_once(
        cyc,
        """        case K_JSR: {
            uint16_t target = (uint16_t)(insn_byte_at(p, &at, 1) | insn_byte_at(p, &at, 2) << 8);
            succ[n++] = target;
            succ[n++] = (uint16_t)(next + p->inline_jsr[target]);  /* return address, past inline arguments */
            break;
        }""",
        """        case K_JSR: {
            uint16_t target = (uint16_t)(insn_byte_at(p, &at, 1) | insn_byte_at(p, &at, 2) << 8);
            succ[n++] = target;
            if (!p->inline_jmp[target])
                succ[n++] = (uint16_t)(next + p->inline_jsr[target]);  /* return address, past inline arguments */
            break;
        }""",
    )
    replace_once(
        cyc,
        """    case K_JSR: {
        uint16_t target = (uint16_t)(b[1] | b[2] << 8);
        if (!live) { out[n] = target; kind[n++] = 1; }
        out[n] = (uint16_t)(next + (live ? 0 : p->inline_jsr[target])); kind[n++] = 0;
        break;
    }""",
        """    case K_JSR: {
        uint16_t target = (uint16_t)(b[1] | b[2] << 8);
        if (!live) { out[n] = target; kind[n++] = 1; }
        if (live || !p->inline_jmp[target])
            out[n] = (uint16_t)(next + (live ? 0 : p->inline_jsr[target])), kind[n++] = 0;
        break;
    }""",
    )
    replace_once(
        cyc,
        """    for (int i = 0; i < cfg->cycle_inline_jsr_count; i++)
        p->inline_jsr[cfg->cycle_inline_jsr[i].target] = cfg->cycle_inline_jsr[i].bytes;""",
        """    for (int i = 0; i < cfg->cycle_inline_jsr_count; i++)
        p->inline_jsr[cfg->cycle_inline_jsr[i].target] = cfg->cycle_inline_jsr[i].bytes;
    for (int i = 0; i < cfg->cycle_inline_jmp_count; i++)
        p->inline_jmp[cfg->cycle_inline_jmp[i].target] = cfg->cycle_inline_jmp[i].bytes;""",
    )

    source_h = cfg_h.read_text(encoding="utf-8")
    source_c = cfg_c.read_text(encoding="utf-8")
    source_cyc = cyc.read_text(encoding="utf-8")
    if "cycle_inline_jmp_count" not in source_h or 'toml_array_in(game, "cycle_inline_jmp")' not in source_c:
        raise SystemExit("ERROR: inline-jump config parser was not installed")
    if "if (!p->inline_jmp[target])" not in source_cyc:
        raise SystemExit("ERROR: inline-jump CFG rule was not installed")
    print("Applied temporary configurable inline tail-dispatch annotation.")
    print("The dispatcher is still a successor; its inline data is no longer treated as fall-through code.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
