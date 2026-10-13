#!/usr/bin/env bash
# S018: owner reported S017 minimal black-screen freeze, canonical returned
# Aurora without observable pass/error. This one emits independent XAM toasts.
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
TC="${OPENXECHAIN_PREFIX:-$ROOT/.openxechain/sysroot}"
OUT="${SMB360_S018_WORK:-$ROOT/build-s018-status}"
cd "$ROOT"
python3 tools/verify_repository.py
test -x "$TC/bin/clang"
test -x "$TC/bin/synthxex"
mkdir -p "$OUT"
export PATH="$TC/bin:$PATH"
export C_INCLUDE_PATH= CPLUS_INCLUDE_PATH= LIBRARY_PATH=
"$TC/bin/clang" -O2 -std=c99 -Wall -Wextra -Werror \
    -ffreestanding -fno-builtin -nostdlib -Isrc/canonical \
    -Wl,/entry:smb360_entry \
    -Wl,/nodefaultlib:libc.a -Wl,/defaultlib:xecorelib.a \
    src/canonical/*.c \
    src/platform/canonical_xex/native_crtless_memory.c \
    src/platform/canonical_xex/native_crtless_status.c \
    src/platform/canonical_xex/embedded_rom.S \
    -o "$OUT/status.exe"
"$TC/bin/synthxex" -i "$OUT/status.exe" \
    -o "$OUT/default.xex" -t title
python3 tools/verify_crtless_xex.py \
    --pe "$OUT/status.exe" --xex "$OUT/default.xex" \
    --embedded-rom assets/canonical/SMB_v026.nes \
    --allow-notifications
(cd "$OUT" && sha256sum status.exe default.xex > SHA256SUMS.txt)
cp docs/TESTE_FISICO_S018.md "$OUT/README_TESTE_S018.md"
cat > "$OUT/BUILD_MANIFEST.txt" <<EOF
kind=S018_CANONICAL_VISIBLE_STATUS_UNTESTED
source_commit=$(git rev-parse HEAD)
rom_source=assets/canonical/SMB_v026.nes
rom_sha256=57fb4ee14288853bc0c8a5a629a103e51e87a4ebae1cc699b435060d2690b047
cpu=POWERPC_XBOX360_CANONICAL_C99_6502_INTERPRETER
entry=smb360_entry_NO_NEWLIB_CRT
video_backend=XAM_NOTIFICATION_ONLY_NOT_MARIO_GRAPHICS
status_states=START_ROM_INVALID_CPU_STOP_FRAME_TIMEOUT_120_FRAMES_PASS
status_import=XNotifyQueueUI
cpu_frames_requested=120
audio_backend=NONE
hardware_test=NOT_TESTED_S018
physical_S017_01=BLACK_SCREEN_FREEZE
physical_S017_02=BLACK_SCREEN_AUTOMATIC_AURORA_RETURN_UNKNOWN_REASON
EOF
echo "S018 native Xbox XEX2 with exact original ROM and XAM stage status: BUILD PASS, hardware NOT TESTED"
