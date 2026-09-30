#!/usr/bin/env python3
"""Extract and fingerprint the owner-supplied SMB_v026 iNES ROM."""
from pathlib import Path
import argparse, hashlib, json, struct

EXPECTED_ROM_SHA256="57fb4ee14288853bc0c8a5a629a103e51e87a4ebae1cc699b435060d2690b047"
EXPECTED_PRG_SHA256="9f5b4f73bde569269645e154a1c8e43308afb2f156baa76e977296d7ca4ed9a4"
EXPECTED_CHR_SHA256="5e22a5c60aef64263ac7b17997479dfdad23389d1f0756c224412c8f7a5535d0"

def sha(b): return hashlib.sha256(b).hexdigest()

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument("rom", type=Path)
    ap.add_argument("--out", type=Path, default=Path("build/rom"))
    a=ap.parse_args()
    raw=a.rom.read_bytes()
    if sha(raw)!=EXPECTED_ROM_SHA256: raise SystemExit("ROM SHA-256 does not match canonical SMB_v026")
    if raw[:4]!=b"NES\x1a": raise SystemExit("invalid iNES header")
    prg_n,chr_n=raw[4],raw[5]
    flags6,flags7=raw[6],raw[7]
    trainer=bool(flags6&4)
    mapper=(flags6>>4)|(flags7&0xF0)
    off=16+(512 if trainer else 0)
    prg=raw[off:off+prg_n*16384]; off+=len(prg)
    chr_=raw[off:off+chr_n*8192]
    if sha(prg)!=EXPECTED_PRG_SHA256 or sha(chr_)!=EXPECTED_CHR_SHA256:
        raise SystemExit("canonical PRG/CHR fingerprint mismatch")
    nmi,reset,irq=struct.unpack("<HHH",prg[-6:])
    a.out.mkdir(parents=True,exist_ok=True)
    (a.out/"SMB_v026.prg").write_bytes(prg)
    (a.out/"SMB_v026.chr").write_bytes(chr_)
    meta={"rom_sha256":sha(raw),"prg_sha256":sha(prg),"chr_sha256":sha(chr_),
          "prg_bytes":len(prg),"chr_bytes":len(chr_),"mapper":mapper,
          "mirroring":"vertical" if flags6&1 else "horizontal","trainer":trainer,
          "vectors":{"nmi":f"0x{nmi:04X}","reset":f"0x{reset:04X}","irq_brk":f"0x{irq:04X}"}}
    (a.out/"manifest.json").write_text(json.dumps(meta,indent=2)+"\n")
    print(json.dumps(meta,indent=2))

if __name__=="__main__": main()
