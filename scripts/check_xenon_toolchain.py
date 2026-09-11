#!/usr/bin/env python3
import os, sys
from pathlib import Path

def fail(msg):
    print(f'xenon toolchain preflight: FAIL: {msg}', file=sys.stderr); return 1

def main():
    root_s=os.environ.get('DEVKITXENON','').strip()
    if not root_s: return fail('DEVKITXENON is not set')
    root=Path(root_s)
    required_files=[root/'rules', root/'app.lds']
    for p in required_files:
        if not p.is_file(): return fail(f'missing {p}')
    search_dirs=[root/'bin', root/'usr/bin']
    tools=['xenon-gcc','xenon-g++','xenon-ar','xenon-objcopy']
    for tool in tools:
        if not any((d/tool).is_file() and os.access(d/tool, os.X_OK) for d in search_dirs):
            return fail(f'missing executable {tool} under DEVKITXENON')
    libdirs=[root/'usr/lib', root/'xenon/lib/32']
    if not any((d/'libxenon.a').is_file() for d in libdirs): return fail('libxenon.a not found')
    if not any((d/'libfat.a').is_file() for d in libdirs): return fail('libfat.a not found')
    print(f'xenon toolchain preflight: PASS ({root})')
    return 0
if __name__=='__main__': raise SystemExit(main())
