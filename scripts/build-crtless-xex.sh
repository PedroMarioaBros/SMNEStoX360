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
cp docs/TESTE_FISICO_S017.md "$OUT/README_S017.txt"
echo "S017: native PowerPC, XEX2, NO Newlib CRT, no ROM; build PASS, hardware NOT TESTED"
