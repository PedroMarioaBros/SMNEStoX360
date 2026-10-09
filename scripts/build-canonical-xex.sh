#!/usr/bin/env bash
# CANONICAL diagnostic build: never includes nathsou, legacy video_fb or audio_xex.
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
TC="${OPENXECHAIN_PREFIX:-$ROOT/.openxechain/sysroot}"
OUT="${SMB360_CANONICAL_XEX_WORK:-$ROOT/build-canonical-xex}"
cd "$ROOT"
python3 tools/verify_repository.py
python3 - <<'PY'
import hashlib
from pathlib import Path
data=Path("assets/canonical/SMB_v026.nes").read_bytes()
expected=(
  ("ROM",data,"57fb4ee14288853bc0c8a5a629a103e51e87a4ebae1cc699b435060d2690b047"),
  ("PRG",data[16:32784],"9f5b4f73bde569269645e154a1c8e43308afb2f156baa76e977296d7ca4ed9a4"),
  ("CHR",data[32784:],"5e22a5c60aef64263ac7b17997479dfdad23389d1f0756c224412c8f7a5535d0"))
assert len(data)==40976, "ROM size mismatch"
for name,part,digest in expected:
    found=hashlib.sha256(part).hexdigest()
    if found!=digest: raise SystemExit(f"{name} SHA-256 mismatch: {found}")
    print(f"S014 {name} SHA-256: {found}")
PY
test -x "$TC/bin/clang" || { echo "Missing OpenXeChain clang: $TC" >&2; exit 2; }
test -x "$TC/bin/synthxex" || { echo "Missing OpenXeChain synthxex: $TC" >&2; exit 2; }
mkdir -p "$OUT"
export PATH="$TC/bin:$PATH"
export C_INCLUDE_PATH= CPLUS_INCLUDE_PATH= LIBRARY_PATH=
src=(src/canonical/*.c src/platform/canonical_xex/main.c src/platform/canonical_xex/embedded_rom.S)
echo "S014 compiler target:"
"$TC/bin/clang" --version | head -4
"$TC/bin/clang" -O2 -std=c99 -Wall -Wextra -Werror \
  -Isrc/canonical "${src[@]}" -o "$OUT/canonical.exe"
test -s "$OUT/canonical.exe"
"$TC/bin/synthxex" -i "$OUT/canonical.exe" -o "$OUT/default.xex" -t title
test -s "$OUT/default.xex"
test "$(dd if="$OUT/default.xex" bs=1 count=4 2>/dev/null)" = "XEX2"
# Retain machine-readable metadata that explicitly labels this as a diagnostic.
sha256sum "$OUT/canonical.exe" "$OUT/default.xex" > "$OUT/SHA256SUMS.txt"
cat > "$OUT/BUILD_MANIFEST.txt" <<EOF
build_kind=CANONICAL_DIAGNOSTIC_NOT_PLAYABLE
project_commit=$(git rev-parse HEAD)
rom_source=assets/canonical/SMB_v026.nes
rom_sha256=57fb4ee14288853bc0c8a5a629a103e51e87a4ebae1cc699b435060d2690b047
prg_sha256=9f5b4f73bde569269645e154a1c8e43308afb2f156baa76e977296d7ca4ed9a4
chr_sha256=5e22a5c60aef64263ac7b17997479dfdad23389d1f0756c224412c8f7a5535d0
source_tree=src/canonical/*.c
entry=src/platform/canonical_xex/main.c
embedded_rom=src/platform/canonical_xex/embedded_rom.S
cpu_backend=6502_instruction_interpreter_on_PowerPC
video_backend=NONE_SAFE_HEADLESS
audio_backend=NONE
xbox_hardware_validation=NOT_TESTED
EOF
echo "S014 canonical diagnostic XEX build PASS (build only, not hardware boot)"
