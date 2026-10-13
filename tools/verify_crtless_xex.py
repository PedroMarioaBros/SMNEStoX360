#!/usr/bin/env python3
"""Static verification of S017's *CRT-free* PowerPC XEX.
This validates the binary image and its imports; it NEVER asserts real
Xbox boot or compatibility with every software exploit/patcher.
"""
import argparse
import hashlib
import struct
from pathlib import Path

ALLOWED = {"KeDelayExecutionThread", "XamLoaderTerminateTitle",
           "ExTerminateThread"}

def u16(blob, offset):
    return struct.unpack_from("<H", blob, offset)[0]

def u32(blob, offset):
    return struct.unpack_from("<I", blob, offset)[0]

def cstr(data, start):
    end = data.index(b"\x00", start)
    return data[start:end].decode("ascii")

def check(pe, xex, rom=None, allow_notifications=False):
    if pe[:2] != b"MZ":
        raise ValueError("not a PE image")
    nt = u32(pe, 0x3c)
    if pe[nt:nt+4] != b"PE\x00\x00" or u16(pe, nt+4) != 0x01f2:
        raise ValueError("not the expected Xbox 360 PowerPC PE")
    nsec = u16(pe, nt+6)
    opt = nt+24
    optsize = u16(pe, nt+20)
    if u16(pe, opt) != 0x010b:
        raise ValueError("expected PE32")
    entry = u32(pe, opt+16)
    imagebase = u32(pe, opt+28)
    subsystem = u16(pe, opt+68)
    if not entry or imagebase != 0x82000000 or subsystem != 14:
        raise ValueError("unexpected PE entry, imagebase, or Xbox subsystem")
    if xex[:4] != b"XEX2":
        raise ValueError("not XEX2")
    nheaders = struct.unpack_from(">I", xex, 20)[0]
    if nheaders < 3 or nheaders > 100:
        raise ValueError("unexpected XEX header count")
    headers = {}
    for i in range(nheaders):
        key, value = struct.unpack_from(">II", xex, 24 + 8*i)
        headers[key] = value
    if headers.get(0x00010100) != imagebase+entry:
        raise ValueError("XEX entry disagrees with PE entry")
    if len(pe) > 0x100000 or len(xex) > 0x100000:
        raise ValueError("unexpectedly big CRT-less title")
    if rom is None:
        if b"NES\x1a" in pe:
            raise ValueError("minimal XEX unexpectedly embeds NES ROM header")
    else:
        if pe.count(rom) != 1:
            raise ValueError("canonical PRG/CHR ROM must be embedded exactly once")
    secbase = opt+optsize
    sections = []
    for i in range(nsec):
        p = secbase + 40*i
        name = pe[p:p+8].split(b"\x00",1)[0]
        sections.append((name,u32(pe,p+12),u32(pe,p+8),u32(pe,p+20),u32(pe,p+16)))
    def fileoff(rva):
        for _,vir,vs,raw,rs in sections:
            if vir <= rva < vir + max(vs,rs) and rva - vir < rs:
                return raw+(rva-vir)
        raise ValueError(f"bad import RVA {rva:#x}")
    # PE32 data directories: import is directory 1.
    imp_rva = u32(pe,opt+96+8)
    imp_size = u32(pe,opt+96+12)
    if not imp_rva or not imp_size:
        raise ValueError("no Xbox kernel/XAM import table")
    names = set()
    libraries = set()
    for di in range(64):
        d = fileoff(imp_rva)+di*20
        lookup, stamp, fwd, dllrva, addr = struct.unpack_from("<IIIII",pe,d)
        if not (lookup or stamp or fwd or dllrva or addr):
            break
        libraries.add(cstr(pe,fileoff(dllrva)).split("@")[0].lower())
        thunk = lookup or addr
        for i in range(128):
            raw = u32(pe,fileoff(thunk + i*4))
            if raw == 0:
                break
            if raw & 0x80000000:
                raise ValueError(f"ordinal import {raw:#x} requires inspection")
            names.add(cstr(pe,fileoff(raw)+2))
        else:
            raise ValueError("unbounded thunk list")
    else:
        raise ValueError("unbounded import modules")
    allowed = ALLOWED | ({"XNotifyQueueUI"} if allow_notifications else set())
    if names != allowed or libraries != {"xboxkrnl.exe","xam.xex"}:
        raise ValueError(f"forbidden/missing imports: {sorted(names)}; modules: {sorted(libraries)}")
    forbidden_names = [b"XamLoaderLaunchTitle", b"CreateFileA",
                       b"WriteFile", b"ReadFile", b"XamInputGetState",
                       b"printf", b"SMB_v026.nes"]
    if not allow_notifications:
        forbidden_names.append(b"XNotifyQueueUI")
    for forbidden in forbidden_names:
        if forbidden in pe:
            raise ValueError(f"CRT or game code accidentally linked: {forbidden!r}")
    print("S017 PE/XEX2: PowerPC machine=0x01f2; subsystem=XBOX")
    print(f"S017 PE entry=0x{imagebase+entry:08x}, matches XEX optional header")
    print(f"S017 import modules={sorted(libraries)}; EXACT APIs={sorted(names)}")
    print(f"S017 size PE={len(pe)} XEX={len(xex)}; NO Newlib startup/file syscalls: PASS")
    print(f"S017 XEX sha256={hashlib.sha256(xex).hexdigest()}")
    print("S017 Xbox hardware test: NOT TESTED. Old S015: FATAL CRASH.")
def main():
    p=argparse.ArgumentParser()
    p.add_argument("--pe",required=True)
    p.add_argument("--xex",required=True)
    p.add_argument("--embedded-rom")
    p.add_argument("--allow-notifications", action="store_true", help="only for the distinct S018 XAM diagnostic; require exact extra XNotifyQueueUI import")
    a=p.parse_args()
    rom=Path(a.embedded_rom).read_bytes() if a.embedded_rom else None
    if rom is not None and hashlib.sha256(rom).hexdigest() != "57fb4ee14288853bc0c8a5a629a103e51e87a4ebae1cc699b435060d2690b047":
        raise ValueError("canonical ROM SHA-256 mismatch")
    check(Path(a.pe).read_bytes(),Path(a.xex).read_bytes(),rom,a.allow_notifications)
if __name__ == "__main__":
    main()
