#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
TC="${OPENXECHAIN_PREFIX:-/workspace/openxechain-cache/sysroot}"
WORK="${SMB360_XEX_WORK:-$ROOT/build-xex}"
UPSTREAM="$WORK/nathsou"
OUT="$WORK/out"
PIN="2143a91e84a7aa888a31f0def9ed5f747fc3da26"

if [ ! -x "$TC/bin/clang" ] || [ ! -x "$TC/bin/synthxex" ]; then
  echo "OpenXeChain toolchain not found at $TC" >&2
  exit 2
fi

mkdir -p "$WORK" "$OUT"
git config --global url."https://github.com/".insteadOf git@github.com:
if [ ! -d "$UPSTREAM/.git" ]; then
  git clone https://github.com/nathsou/smb.git "$UPSTREAM"
fi
git -C "$UPSTREAM" fetch --prune origin
git -C "$UPSTREAM" checkout --detach "$PIN"
git -C "$UPSTREAM" reset --hard "$PIN"
git -C "$UPSTREAM" clean -fdx
test "$(git -C "$UPSTREAM" rev-parse HEAD)" = "$PIN"

# Xenon is big-endian. Make the palette-cache key byte-order independent.
git -C "$UPSTREAM" apply "$ROOT/patches/nathsou-ppu-endian.patch"

# C99 plain-inline public functions do not emit cross-TU definitions under
# Clang/GCC. These two functions are public API and are called from smb_main.c.
sed -i 's/^inline void update_controller1(/void update_controller1(/' "$UPSTREAM/codegen/lib/cpu.c"
sed -i 's/^inline void next_frame(/void next_frame(/' "$UPSTREAM/codegen/lib/cpu.c"

grep -q '^void update_controller1(' "$UPSTREAM/codegen/lib/cpu.c"
grep -q '^void next_frame(' "$UPSTREAM/codegen/lib/cpu.c"

LIB="$UPSTREAM/codegen/lib"
PE="$OUT/smb360.exe"
XEX="$OUT/default.xex"

export PATH="$TC/bin:$PATH"
export C_INCLUDE_PATH=
export CPLUS_INCLUDE_PATH=
export LIBRARY_PATH=

SOURCES=(
  "$LIB/instructions.c"
  "$LIB/code.c"
  "$LIB/data.c"
  "$LIB/cpu.c"
  "$LIB/ppu.c"
  "$LIB/apu.c"
  "$LIB/common.c"
  "$ROOT/src/platform/xex/video_fb.c"
  "$ROOT/src/platform/xex/smb_main.c"
)

"$TC/bin/clang" \
  -O2 -std=gnu11 -Wall -Wextra \
  -I"$LIB" -I"$ROOT/src/platform/xex" \
  "${SOURCES[@]}" \
  -o "$PE"

test -s "$PE"
"$TC/bin/synthxex" -i "$PE" -o "$XEX" -t title
test -s "$XEX"
magic="$(dd if="$XEX" bs=1 count=4 2>/dev/null)"
test "$magic" = "XEX2"

sha256sum "$PE" "$XEX" | tee "$OUT/SHA256SUMS.txt"
printf 'project_commit=%s\nnathsou_commit=%s\nopenxechain_prefix=%s\nartifact=default.xex\n' \
  "$(git -C "$ROOT" rev-parse HEAD)" "$PIN" "$TC" > "$OUT/BUILD_MANIFEST.txt"

printf 'SMB360 XEX build PASS: %s\n' "$XEX"
