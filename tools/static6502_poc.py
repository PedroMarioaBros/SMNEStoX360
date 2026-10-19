#!/usr/bin/env python3
"""Small, fail-closed ROM-specific 6502 -> C static-translation proof of concept.

This is not a full recompilation. It translates a bounded straight-line subset
from the canonical ROM into direct C statements; no opcode dispatcher runs at
runtime. Unknown instructions stop translation explicitly.
"""
import argparse
import hashlib
import os
import pathlib
import subprocess
import tempfile

ROM_SHA256 = "57fb4ee14288853bc0c8a5a629a103e51e87a4ebae1cc699b435060d2690b047"
# opcode: (mnemonic, byte length, C statement template)
OPS = {
    0x78: ("SEI", 1, "c->p |= F_I;"),
    0xD8: ("CLD", 1, "c->p &= (uint8_t)~F_D;"),
    0xA9: ("LDA #imm", 2, "c->a = IMM; set_nz(c, c->a);"),
    0xA2: ("LDX #imm", 2, "c->x = IMM; set_nz(c, c->x);"),
    0xA0: ("LDY #imm", 2, "c->y = IMM; set_nz(c, c->y);"),
    0x8D: ("STA abs", 3, "bus_write(c, ABS, c->a);"),
    0xAD: ("LDA abs", 3, "c->a = bus_read(c, ABS); set_nz(c, c->a);"),
    0x10: ("BPL rel", 2, "BRANCH_BPL"),
    0x8E: ("STX abs", 3, "bus_write(c, ABS, c->x);"),
    0x8C: ("STY abs", 3, "bus_write(c, ABS, c->y);"),
    0x9A: ("TXS", 1, "c->s = c->x;"),
    0xAA: ("TAX", 1, "c->x = c->a; set_nz(c, c->x);"),
    0xA8: ("TAY", 1, "c->y = c->a; set_nz(c, c->y);"),
    0xE8: ("INX", 1, "c->x++; set_nz(c, c->x);"),
    0xCA: ("DEX", 1, "c->x--; set_nz(c, c->x);"),
    0xC8: ("INY", 1, "c->y++; set_nz(c, c->y);"),
    0x88: ("DEY", 1, "c->y--; set_nz(c, c->y);"),
    0x18: ("CLC", 1, "c->p &= (uint8_t)~F_C;"),
    0x38: ("SEC", 1, "c->p |= F_C;"),
    0xEA: ("NOP", 1, "/* NOP */"),
}
HEADER = """#ifndef SMB360_STATIC6502_POC_H
#define SMB360_STATIC6502_POC_H
#include <stdint.h>
enum { F_C=1, F_Z=2, F_I=4, F_D=8, F_B=16, F_U=32, F_V=64, F_N=128 };
typedef struct {
 uint8_t a,x,y,s,p;
 void *opaque;
 uint8_t (*read)(void *, uint16_t);
 void (*write)(void *, uint16_t, uint8_t);
} smb_static_cpu;
void smb_static_poc_entry(smb_static_cpu *c);
#endif
"""
PRELUDE = """#include "static6502_poc.h"
static void set_nz(smb_static_cpu *c, uint8_t v) {
 c->p = (uint8_t)((c->p & (uint8_t)~(F_N|F_Z)) | (v ? 0 : F_Z) | (v & F_N));
}
static uint8_t bus_read(smb_static_cpu *c, uint16_t a) {
 return c->read ? c->read(c->opaque, a) : 0;
}
static void bus_write(smb_static_cpu *c, uint16_t a, uint8_t v) {
 if (c->write) c->write(c->opaque, a, v);
}
"""

def get_prg(path):
    data = pathlib.Path(path).read_bytes()
    digest = hashlib.sha256(data).hexdigest()
    if digest != ROM_SHA256:
        raise SystemExit("canonical ROM SHA-256 mismatch: " + digest)
    if len(data) != 40976 or data[:4] != b"NES\x1a" or data[4:6] != bytes((2, 1)):
        raise SystemExit("expected canonical iNES ROM: 40976 bytes, 2 PRG banks, 1 CHR bank")
    return data[16:16 + 32768]

def translate(prg, start, count):
    if not 0x8000 <= start <= 0xFFFF or count < 1:
        raise ValueError("start must be $8000-$FFFF and instruction count must be positive")
    pc, records = start, []
    branch_tests = {0x10: "!(c->p & F_N)"}
    for _ in range(count):
        off = pc - 0x8000
        if off >= len(prg):
            raise ValueError("PC outside canonical PRG at $%04X" % pc)
        op = prg[off]
        if op not in OPS:
            raise ValueError("unsupported opcode $%02X at $%04X; stopped without fallback" % (op, pc))
        name, size, template = OPS[op]
        raw = prg[off:off + size]
        if len(raw) != size:
            raise ValueError("truncated instruction at $%04X" % pc)
        imm = raw[1] if size == 2 else 0
        addr = raw[1] | (raw[2] << 8) if size == 3 else 0
        target = None
        if op in branch_tests:
            disp = raw[1] - 256 if raw[1] & 0x80 else raw[1]
            target = (pc + size + disp) & 0xFFFF
            stmt = "if (%s) goto L_%04X;" % (branch_tests[op], target)
        else:
            stmt = template.replace("IMM", "0x%02Xu" % imm).replace("ABS", "0x%04Xu" % addr)
        records.append({"pc": pc, "raw": raw, "name": name, "stmt": stmt, "target": target})
        pc = (pc + size) & 0xFFFF
    pcs = {r["pc"] for r in records}
    for r in records:
        if r["target"] is not None and r["target"] not in pcs:
            raise ValueError("branch at $%04X targets $%04X outside translated span" % (r["pc"], r["target"]))
    targets = {r["target"] for r in records if r["target"] is not None}
    lines, listing = [], []
    for r in records:
        if r["pc"] in targets:
            lines.append("L_%04X:" % r["pc"])
        lines.append("    /* $%04X: %s %s */\\n    %s" % (r["pc"], " ".join("%02X" % b for b in r["raw"]), r["name"], r["stmt"]))
        listing.append((r["pc"], r["raw"].hex(), r["name"], r["target"]))
    return "\\n".join(lines), listing, pc

def self_test(rom):
    prg = get_prg(rom)
    body, listing, end = translate(prg, 0x8000, 8)
    if end != 0x800F or len(listing) != 8 or "goto L_800A" not in body:
        raise SystemExit("unexpected static span/branch translation")
    try:
        translate(prg, 0x800D, 1)
    except ValueError as exc:
        if "branch at $800D targets $800A outside translated span" not in str(exc):
            raise
    else:
        raise SystemExit("fail-closed test failed: external branch target accepted")
    harness = """#include <assert.h>
#include "static6502_poc.h"
static int writes, reads; static uint16_t address; static uint8_t value;
static uint8_t rd(void *o, uint16_t a) { (void)o; assert(a==0x2002); return reads++ ? 0x80 : 0x00; }
static void wr(void *o, uint16_t a, uint8_t v) { (void)o; writes++; address=a; value=v; }
int main(void) {
 smb_static_cpu c = {0}; c.p=F_D|F_C; c.read=rd; c.write=wr; smb_static_poc_entry(&c);
 assert(c.a==0x80 && c.x==0xFF && c.s==0xFF);
 assert((c.p&F_I)!=0 && (c.p&F_D)==0 && (c.p&F_Z)==0 && (c.p&F_N)!=0);
 assert(writes==1 && address==0x2000 && value==0x10 && reads==2);
 return 0;
}
"""
    with tempfile.TemporaryDirectory(prefix="smb360-static-poc-") as td:
        d = pathlib.Path(td)
        (d / "static6502_poc.h").write_text(HEADER)
        (d / "generated.c").write_text(PRELUDE + "\\nvoid smb_static_poc_entry(smb_static_cpu *c) {\\n" + body + "\\n}\\n")
        (d / "test.c").write_text(harness)
        exe = d / "test"
        subprocess.run([os.environ.get("CC", "cc"), "-std=c99", "-Wall", "-Wextra",
                        "-Werror", "-pedantic", str(d / "generated.c"), str(d / "test.c"),
                        "-I", td, "-o", str(exe)], check=True)
        subprocess.run([str(exe)], check=True)
    print("PASS: canonical ROM hash verified; eight ROM-derived instructions, including a backward BPL branch, statically lowered;")
    print("      generated C compiled with strict warnings; read/write callbacks and branch execution assertions passed.")
    print("      out-of-span branch rejection passed. Span $8000-$800E; next PC $%04X." % end)
    print("LIMIT: small block only; not full decompilation, gameplay equivalence, or Xbox build.")

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--rom", default="assets/canonical/SMB_v026.nes")
    ap.add_argument("--start", type=lambda x: int(x, 0), default=0x8000)
    ap.add_argument("--instructions", type=int, default=6)
    ap.add_argument("--out-c", type=pathlib.Path)
    ap.add_argument("--self-test", action="store_true")
    args = ap.parse_args()
    if args.self_test:
        self_test(args.rom)
        return
    prg = get_prg(args.rom)
    body, listing, end = translate(prg, args.start, args.instructions)
    generated = PRELUDE + "\nvoid smb_static_poc_entry(smb_static_cpu *c) {\n" + body + "\n}\n"
    if args.out_c:
        args.out_c.parent.mkdir(parents=True, exist_ok=True)
        args.out_c.write_text(generated)
    else:
        print(generated)
    print("ROM-derived listing:", listing, "next PC: 0x%04X" % end, file=__import__("sys").stderr)

if __name__ == "__main__":
    main()
