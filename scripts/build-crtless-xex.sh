#!/usr/bin/env bash
# S017: Xbox OS / PowerPC XEX from an explicit entrypoint WITHOUT Newlib CRT.
# Failure of S015 on REAL Xbox documented in issue #12.
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
TC="${OPENXECHAIN_PREFIX:-$ROOT/.openxechain/sysroot}"
OUT="${SMB360_CRTLESS_WORK:-$ROOT/build-crtless-xex}"
cd "$ROOT"
python3 tools/verify_repository.py
test -x "$TC/bin/clang"
test -x "$TC/bin/synthxex"
mkdir -p "$OUT"
export PATH="$TC/bin:$PATH"
export C_INCLUDE_PATH= CPLUS_INCLUDE_PATH= LIBRARY_PATH=
# Critical: explicit Xbox-OS entrypoint, NOT 'main'; do not link the
# default libc CRT start or Newlib file-system syscalls.
"$TC/bin/clang" -O2 -std=c99 -Wall -Wextra -Werror \
  -ffreestanding -fno-builtin -nostdlib \
  -Wl,/entry:smb360_entry \
  -Wl,/nodefaultlib:libc.a -Wl,/defaultlib:xecorelib.a \
  src/platform/canonical_xex/native_crtless_probe.c \
  -o "$OUT/crtless.exe"
test -s "$OUT/crtless.exe"
"$TC/bin/synthxex" -i "$OUT/crtless.exe" -o "$OUT/default.xex" -t title
test -s "$OUT/default.xex"
python3 tools/verify_crtless_xex.py --pe "$OUT/crtless.exe" --xex "$OUT/default.xex"
(
  cd "$OUT"
  sha256sum crtless.exe default.xex > SHA256SUMS.txt
)
cat > "$OUT/BUILD_MANIFEST.txt" <<EOF
kind=S017_NATIVE_CRTLESS_XBOX_OS_BOOT_DIAGNOSTIC
source_commit=$(git rev-parse HEAD)
cpu=POWERPC_XBOX360
entry=smb360_entry_NO_NEWLIB_CRT
source=src/platform/canonical_xex/native_crtless_probe.c
rom_embedded=NO_THIS_IS_A_BOOT_DIAGNOSTIC_NOT_THE_GAME
allowed_apis=KeDelayExecutionThread,XamLoaderTerminateTitle,ExTerminateThread
xex_generator=OPENXECHAIN_SYNTHXEX
runtime_test=NOT_TESTED_ON_XBOX_HARDWARE
prior_hardware_result=S015_FATAL_CRASH
EOF
# Stage 02: same explicit title entry, but now with the owner's original ROM
# and canonical CPU+PPU.  It is NOT a game (no Xbox video/audio yet).
mkdir -p "$OUT/02_CANONICAL_CRTLESS"
"$TC/bin/clang" -O2 -std=c99 -Wall -Wextra -Werror \
  -ffreestanding -fno-builtin -nostdlib -Isrc/canonical \
  -Wl,/entry:smb360_entry \
  -Wl,/nodefaultlib:libc.a -Wl,/defaultlib:xecorelib.a \
  src/canonical/*.c \
  src/platform/canonical_xex/native_crtless_memory.c \
  src/platform/canonical_xex/native_crtless_canonical.c \
  src/platform/canonical_xex/embedded_rom.S \
  -o "$OUT/02_CANONICAL_CRTLESS/canonical.exe"
"$TC/bin/synthxex" -i "$OUT/02_CANONICAL_CRTLESS/canonical.exe" \
  -o "$OUT/02_CANONICAL_CRTLESS/default.xex" -t title
python3 tools/verify_crtless_xex.py \
  --pe "$OUT/02_CANONICAL_CRTLESS/canonical.exe" \
  --xex "$OUT/02_CANONICAL_CRTLESS/default.xex" \
  --embedded-rom assets/canonical/SMB_v026.nes
(
  cd "$OUT/02_CANONICAL_CRTLESS"
  sha256sum canonical.exe default.xex > SHA256SUMS.txt
)
cat > "$OUT/02_CANONICAL_CRTLESS/BUILD_MANIFEST.txt" <<EOF
kind=S017_CRTLESS_CANONICAL_ROM_DIAGNOSTIC_NOT_PLAYABLE
source_commit=$(git rev-parse HEAD)
cpu=POWERPC_XBOX360
entry=smb360_entry_NO_NEWLIB_CRT
rom_embedded=YES_EXACT_SMB_V026
rom_sha256=57fb4ee14288853bc0c8a5a629a103e51e87a4ebae1cc699b435060d2690b047
video_backend=NONE
audio_backend=NONE
runtime_test=NOT_TESTED_ON_XBOX_HARDWARE
run_only_after=01_MINIMAL_CRTLESS_PASS_ON_HARDWARE
EOF
cp docs/TESTE_FISICO_S017.md "$OUT/README_S017.txt"
echo "S017: native PowerPC, XEX2, NO Newlib CRT, no ROM; build PASS, hardware NOT TESTED"
