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
        '#include "hw_internal.h"\n#include "cpu6502.h"\n#include <stdio.h>\nvoid cyc_profile_state_tick(void);\n',
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

/* Targeted context profiler for the four still-unobserved dispatcher families.
 * It records a compact set of unique execution contexts only; it never changes
 * CPU state, RAM, timing, bus activity or the selected dispatcher target. */
typedef struct {
    uint16_t callsite;
    uint8_t selector, slot_x, slot_id, enemy_flag_active, enemy_state_class;
    uint8_t timer_active, starflag_below5, oper_mode, mode_task, world, area;
    uint8_t area_style, area_type;
} CycProfileContextSeen;
#define CYC_PROFILE_MAX_CONTEXTS 256
static CycProfileContextSeen cyc_profile_context_seen[CYC_PROFILE_MAX_CONTEXTS];
static unsigned cyc_profile_context_seen_count;

static void cyc_profile_context(uint16_t callsite, uint8_t selector, const uint8_t *ram) {
    if (callsite != 0x8215 && callsite != 0x9664 &&
        callsite != 0xC27F && callsite != 0xC88F && callsite != 0xC907) return;

    uint8_t slot_x = 0xFF, slot_id = 0xFF, enemy_flag_active = 0, enemy_state_class = 0;
    if (callsite == 0xC27F || callsite == 0xC88F || callsite == 0xC907) {
        if (cpu.x < 6u) {
            slot_x = cpu.x;
            slot_id = ram[0x16u + cpu.x];
            enemy_flag_active = (uint8_t)(ram[0x0Fu + cpu.x] != 0u);
            enemy_state_class = (uint8_t)(ram[0x1Eu + cpu.x] & 0xE0u);
        }
    }

    CycProfileContextSeen context = {
        callsite, selector, slot_x, slot_id, enemy_flag_active, enemy_state_class,
        (uint8_t)(ram[0x0747] != 0u), (uint8_t)(ram[0x0746] < 5u),
        ram[0x0770], ram[0x0772], ram[0x075F], ram[0x075C],
        ram[0x0733], ram[0x074E]
    };
    for (unsigned i = 0; i < cyc_profile_context_seen_count; ++i) {
        const CycProfileContextSeen *seen = &cyc_profile_context_seen[i];
        if (seen->callsite == context.callsite && seen->selector == context.selector &&
            seen->slot_x == context.slot_x && seen->slot_id == context.slot_id &&
            seen->enemy_flag_active == context.enemy_flag_active &&
            seen->enemy_state_class == context.enemy_state_class &&
            seen->timer_active == context.timer_active &&
            seen->starflag_below5 == context.starflag_below5 &&
            seen->oper_mode == context.oper_mode && seen->mode_task == context.mode_task &&
            seen->world == context.world && seen->area == context.area &&
            seen->area_style == context.area_style && seen->area_type == context.area_type) return;
    }
    if (cyc_profile_context_seen_count >= CYC_PROFILE_MAX_CONTEXTS) {
        fprintf(stderr, "[DISPATCH_CONTEXT_ERROR] context cap exceeded\n");
        return;
    }
    cyc_profile_context_seen[cyc_profile_context_seen_count++] = context;
    fprintf(stderr,
        "[DISPATCH_CONTEXT] frame=%u callsite=$%04X selector=%u slot_x=$%02X "
        "enemy_id=$%02X enemy_flag_active=%u enemy_state_class=$%02X "
        "timer0747=$%02X starflag0746=$%02X oper_mode=$%02X mode_task=$%02X "
        "world=$%02X area=$%02X area_style=$%02X area_type=$%02X\n",
        cyc_profile_state_frame, context.callsite, context.selector,
        context.slot_x, context.slot_id, context.enemy_flag_active,
        context.enemy_state_class, ram[0x0747], ram[0x0746], context.oper_mode,
        context.mode_task, context.world, context.area, context.area_style, context.area_type);
}

void cyc_profile_dispatch(uint16_t pc, bool from_interpreter) {
    if (pc != 0x8E04) return;
    const uint8_t *ram = cyc_cpu_ram();
    uint8_t sp = cpu.s;
    uint16_t ret = (uint16_t)(ram[0x100u + ((sp + 1u) & 0xFFu)] |
                              ((uint16_t)ram[0x100u + ((sp + 2u) & 0xFFu)] << 8));
    uint16_t callsite = (uint16_t)(ret - 2u);
    uint8_t selector = cpu.a;
    cyc_profile_context(callsite, selector, ram);
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

    state_hook = r'''
/* Evaluation-only SMB_v026 progress telemetry. Samples RAM at frame boundaries;
 * it does not modify CPU registers, bus activity or emulated cycles. Emits on
 * discrete-state changes and every 60 frames; $0747 is sampled for each change
 * at values <= 5 so the $C987 zero-condition stays observable without flooding
 * the log during the per-frame falling countdown. */
static unsigned cyc_profile_state_frame;
static uint8_t cyc_profile_last_state[17];
static bool cyc_profile_have_state;

void cyc_profile_state_tick(void) {
    const uint8_t *ram = cyc_cpu_ram();
    uint8_t state[17] = {
        ram[0x0770], ram[0x0772], ram[0x000E], ram[0x075F], ram[0x075C],
        ram[0x075A], ram[0x006D], ram[0x0086], ram[0x00CE],
        ram[0x0016], ram[0x0017], ram[0x0018],
        ram[0x0019], ram[0x001A], ram[0x001B],
        ram[0x0746], ram[0x0747]
    };
    /* Keep the countdown byte out of generic change detection: it decrements
     * every frame while falling. We still emit each near-zero value because
     * $C987's candidate path is conditioned on RAM[$0747] == 0. */
    static const uint8_t relevant[] = {0, 1, 2, 3, 4, 5, 9, 10, 11, 12, 13, 14, 15};
    bool counter_condition_changed = cyc_profile_have_state &&
        state[16] != cyc_profile_last_state[16] && state[16] <= 5u;
    bool changed = !cyc_profile_have_state;
    if (!changed) {
        for (unsigned i = 0; i < sizeof(relevant); ++i) {
            unsigned k = relevant[i];
            if (state[k] != cyc_profile_last_state[k]) {
                changed = true;
                break;
            }
        }
    }
    if (changed || counter_condition_changed ||
        (cyc_profile_state_frame % 60u) == 0u) {
        fprintf(stderr,
                "[SMB_STATE] frame=%u mode=$%02X mode_task=$%02X game_task=$%02X "
                "world=$%02X area=$%02X lives=$%02X player_page=$%02X player_x=$%02X "
                "player_y=$%02X cond0746=$%02X cond0747=$%02X "
                "enemy_ids=$%02X,$%02X,$%02X,$%02X,$%02X,$%02X\n",
                cyc_profile_state_frame,
                state[0], state[1], state[2], state[3], state[4], state[5],
                state[6], state[7], state[8], state[15], state[16],
                state[9], state[10], state[11], state[12], state[13], state[14]);
    }
    for (unsigned i = 0; i < sizeof(state); ++i) cyc_profile_last_state[i] = state[i];
    cyc_profile_have_state = true;
    cyc_profile_state_frame++;
}

'''
    replace_once(
        run_c,
        "void cyc_run_frame(void) {\n    hw_frame_done = hw_frame_end_hit = false;",
        "void cyc_run_frame(void) {\n    cyc_profile_state_tick();\n    hw_frame_done = hw_frame_end_hit = false;",
    )

    replace_once(
        run_c,
        "/* One dispatch at an instruction boundary: recompiled ROM code, a compiled\n",
        state_hook + hook + "/* One dispatch at an instruction boundary: recompiled ROM code, a compiled\n",
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
    if "[SMB_STATE]" not in source or "cyc_profile_state_tick();" not in source:
        raise SystemExit("ERROR: SMB state telemetry is missing")
    if "[DISPATCH_CONTEXT]" not in source or "cyc_profile_context(callsite, selector, ram);" not in source:
        raise SystemExit("ERROR: targeted dispatcher context profiling is missing")
    if "callsite != 0xC907" not in source or "callsite == 0xC907" not in source:
        raise SystemExit("ERROR: downstream enemy type context profiling is missing")
    if "counter_condition_changed" not in source or "state[16] <= 5u" not in source:
        raise SystemExit("ERROR: low-value $0747 condition sampling is missing")
    if "cyc_profile_dispatch(cpu.pc, true);" not in interp_file.read_text(encoding="utf-8"):
        raise SystemExit("ERROR: profile hook was not installed in interpreter fetch")
    print("Applied temporary inline-dispatch profile hook to native fetch and interpreter steps.")
    print("Each observed $8E04 entry reports caller, selector, inline table word, and decoded target.")
    print("Reference profiling emits compact [DISPATCH_CONTEXT] records for mode/area/enemy-init/enemy-run/type dispatchers.")
    print("Reference profiling emits [SMB_STATE] on discrete-state changes, every 60 frames, and when RAM[$0747] changes at values <= 5.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
