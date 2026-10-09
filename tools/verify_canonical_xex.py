#!/usr/bin/env python3
"""Validate canonical diagnostic Xbox XEX2 and the entire original ROM in its PE.

Runs on the generated files (not just on the input .nes): checks the machine
architecture, full embedded iNES payload, and SHA-256. No Xbox boot claim.
"""
import argparse
import hashlib
from pathlib import Path
import struct

from extract_canonical_rom import (
    EXPECTED_ROM_SHA256, EXPECTED_PRG_SHA256, EXPECTED_CHR_SHA256,
)

ROOT=Path(__file__).resolve().parents[1]

def validate(pe: Path, xex: Path, rom_path: Path) -> None:
    rom=rom_path.read_bytes()
    if len(rom)!=40976 or hashlib.sha256(rom).hexdigest()!=EXPECTED_ROM_SHA256:
        raise ValueError("ROM length/SHA-256 does not match canonical iNES")
    if hashlib.sha256(rom[16:32784]).hexdigest()!=EXPECTED_PRG_SHA256:
        raise ValueError("PRG SHA-256 mismatch")
    if hashlib.sha256(rom[32784:]).hexdigest()!=EXPECTED_CHR_SHA256:
        raise ValueError("CHR SHA-256 mismatch")
    pe_bytes=pe.read_bytes()
    if len(pe_bytes)<256 or pe_bytes[:2]!=b"MZ":
        raise ValueError("not a PE executable (MZ)")
    pe_offset=struct.unpack_from("<I",pe_bytes,0x3c)[0]
    if pe_offset+24>len(pe_bytes) or pe_bytes[pe_offset:pe_offset+4]!=b"PE\0\0":
        raise ValueError("invalid PE signature")
    machine,sections=struct.unpack_from("<HH",pe_bytes,pe_offset+4)
    if machine not in (0x1f0,0x1f2):
        raise ValueError(f"unexpected PE CPU machine 0x{machine:04x}, expected PowerPC")
    if sections==0 or sections>64:
        raise ValueError(f"invalid PE section count {sections}")
    offset=pe_bytes.find(rom)
    if offset<0:
        raise ValueError("complete canonical 40976-byte iNES ROM absent from PE")
    if pe_bytes.find(rom,offset+1)>=0:
        raise ValueError("ambiguous: ROM payload embedded multiple times")
    xex_bytes=xex.read_bytes()
    if xex_bytes[:4]!=b"XEX2" or len(xex_bytes)<1024:
        raise ValueError("invalid or empty XEX2")
    print(f"CANONICAL XEX PE PowerPC machine=0x{machine:04x} sections={sections}")
    print(f"Embedded ROM bytes={len(rom)} PE raw offset=0x{offset:x}")
    print(f"Original ROM SHA-256={EXPECTED_ROM_SHA256}")
    print(f"PE SHA-256={hashlib.sha256(pe_bytes).hexdigest()}")
    print(f"XEX2 SHA-256={hashlib.sha256(xex_bytes).hexdigest()}")
    print("Canonical PPC PE, full embedded ROM and XEX2: PASS (build only)")

def main() -> None:
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument("--pe",type=Path,default=ROOT/"build-canonical-xex/canonical.exe")
    p.add_argument("--xex",type=Path,default=ROOT/"build-canonical-xex/default.xex")
    p.add_argument("--rom",type=Path,default=ROOT/"assets/canonical/SMB_v026.nes")
    args=p.parse_args()
    try:
        validate(args.pe,args.xex,args.rom)
    except (ValueError, OSError) as e:
        p.error(str(e))

if __name__=="__main__":
    main()
