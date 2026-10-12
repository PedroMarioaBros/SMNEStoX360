#!/usr/bin/env bash
# S016: two controlled BOOT DIAGNOSTICS after real Xbox Fatal Crash #12.
# NO nathsou, no raw GPU framebuffer, no XAM, no print/stdio.
# Not a playable game. Do not ask for test unless exact CI run is GREEN.
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
TC="${OPENXECHAIN_PREFIX:-$ROOT/.openxechain/sysroot}"
OUT="${SMB360_BOOT_ISOLATION_WORK:-$ROOT/build-boot-isolation}"
cd "$ROOT"
python3 tools/verify_repository.py
test -x "$TC/bin/clang"
test -x "$TC/bin/synthxex"
mkdir -p "$OUT/01_KERNEL_ONLY" "$OUT/02_CANONICAL_NO_XAM"
export PATH="$TC/bin:$PATH"
export C_INCLUDE_PATH= CPLUS_INCLUDE_PATH= LIBRARY_PATH=
echo "S016 boot-isolation: minimal Xbox OS kernel, no ROM, no XAM"
"$TC/bin/clang" -O2 -std=c99 -Wall -Wextra -Werror \
 src/platform/canonical_xex/boot_kernel_only.c \
 -o "$OUT/01_KERNEL_ONLY/boot.exe"
"$TC/bin/synthxex" -i "$OUT/01_KERNEL_ONLY/boot.exe" \
 -o "$OUT/01_KERNEL_ONLY/default.xex" -t title
echo "S016 boot-isolation: original SMB_v026 ROM and canonical C99 core, no XAM"
"$TC/bin/clang" -O2 -std=c99 -Wall -Wextra -Werror \
 -Isrc/canonical src/canonical/*.c \
 src/platform/canonical_xex/boot_canonical_no_xam.c \
 src/platform/canonical_xex/embedded_rom.S \
 -o "$OUT/02_CANONICAL_NO_XAM/boot.exe"
"$TC/bin/synthxex" -i "$OUT/02_CANONICAL_NO_XAM/boot.exe" \
 -o "$OUT/02_CANONICAL_NO_XAM/default.xex" -t title
python3 tools/verify_canonical_xex.py \
 --pe "$OUT/02_CANONICAL_NO_XAM/boot.exe" \
 --xex "$OUT/02_CANONICAL_NO_XAM/default.xex"
python3 - "$OUT" <<'PY'
import hashlib
import struct
import sys
from pathlib import Path
out=Path(sys.argv[1])
rom=Path("assets/canonical/SMB_v026.nes").read_bytes()
expected_rom_sha="57fb4ee14288853bc0c8a5a629a103e51e87a4ebae1cc699b435060d2690b047"
assert hashlib.sha256(rom).hexdigest()==expected_rom_sha
for stage in ("01_KERNEL_ONLY","02_CANONICAL_NO_XAM"):
    folder=out/stage
    pe=(folder/"boot.exe").read_bytes()
    xex=(folder/"default.xex").read_bytes()
    assert pe[:2]==b"MZ" and xex[:4]==b"XEX2"
    off=struct.unpack_from("<I",pe,0x3c)[0]
    assert pe[off:off+4]==b"PE\0\0"
    machine=struct.unpack_from("<H",pe,off+4)[0]
    assert machine==0x1f2, f"{stage} machine={machine:#x} not Xbox PowerPC"
    assert b"xam.xex" not in pe.lower(), f"{stage} incorrectly links XAM imports"
    assert b"XNotifyQueueUI" not in pe
    if stage.startswith("01_"):
        assert rom not in pe, "kernel-only stage unexpectedly embeds ROM"
    else:
        assert pe.count(rom)==1, "canonical stage missing full ROM or duplicates"
    print(f"S016 {stage}: PE PowerPC 0x{machine:04x}, no XAM, XEX2, ROM={'absent' if stage.startswith('01_') else 'exactly once'}: PASS")
PY
for stage in 01_KERNEL_ONLY 02_CANONICAL_NO_XAM; do
 (cd "$OUT/$stage" && sha256sum boot.exe default.xex > SHA256SUMS.txt)
 cat > "$OUT/$stage/BUILD_MANIFEST.txt" <<EOF
test_kind=S016_BOOT_ISOLATION_NOT_A_GAME
stage=$stage
project_commit=$(git rev-parse HEAD)
os=XBOX360_DASHBOARD
rom_sha256=57fb4ee14288853bc0c8a5a629a103e51e87a4ebae1cc699b435060d2690b047
game_source=$(if [ "$stage" = "01_KERNEL_ONLY" ]; then echo NONE; else echo ORIGINAL_ROM_AND_CANONICAL_C99; fi)
xam_imports=NONE
direct_gpu_mmio=NONE
stdio=NONE
xbox_boot_status=UNTESTED_NEW_IMAGE_AFTER_S015_FATAL_CRASH
EOF
done
cp docs/TESTE_FISICO_S016.md "$OUT/README_TESTE_S016.md"
echo "S016 BOOT ISOLATION build PASS (real console: UNTESTED)"
