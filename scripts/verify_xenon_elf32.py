#!/usr/bin/env python3
import sys
from pathlib import Path

def fail(msg):
    print(f'xenon ELF32 verification: FAIL: {msg}', file=sys.stderr); return 1

def main():
    if len(sys.argv)!=2:
        print(f'usage: {sys.argv[0]} <file.elf32>', file=sys.stderr); return 2
    p=Path(sys.argv[1])
    try: h=p.read_bytes()[:64]
    except OSError as e: return fail(str(e))
    if len(h)<52: return fail('file is too short to be ELF32')
    if h[:4]!=b'\x7fELF': return fail('bad ELF magic')
    if h[4]!=1: return fail(f'ELF class is {h[4]}, expected ELFCLASS32 (1)')
    if h[5]!=2: return fail(f'ELF data encoding is {h[5]}, expected big-endian (2)')
    machine=int.from_bytes(h[18:20], 'big')
    if machine!=20: return fail(f'e_machine is {machine}, expected EM_PPC (20)')
    etype=int.from_bytes(h[16:18], 'big')
    if etype not in (2,3): return fail(f'e_type is {etype}, expected executable/shared ELF')
    if p.stat().st_size==0: return fail('empty file')
    print(f'xenon ELF32 verification: PASS ({p}, {p.stat().st_size} bytes, PowerPC big-endian ELF32)')
    return 0
if __name__=='__main__': raise SystemExit(main())
