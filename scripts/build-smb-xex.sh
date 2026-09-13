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

# Reject upstream drift before applying local audited patches.
while read -r expected path; do
  actual="$(git -C "$UPSTREAM" hash-object "$path")"
  if [ "$actual" != "$expected" ]; then
    echo "nathsou blob mismatch: $path expected=$expected actual=$actual" >&2
    exit 3
  fi
done <<'EOF'
d645695673349e3947e8e5ae42332d0ac3164cd7 LICENSE
e94ae6adc038094ddf953727865a2d06097f142e codegen/lib/apu.c
fe70e037f595b7dec54fd0846e6c54620698ab50 codegen/lib/apu.h
779a38e8cd8171a1dff1e21a835a5609f08eea07 codegen/lib/code.c
91820dccd716db733f0842b04234d62adaac1d66 codegen/lib/code.h
6f605b4dede3b331f345a8a3b67d8085a53e5534 codegen/lib/common.c
aabda92251d703985080473e7c12a7444c6befa2 codegen/lib/common.h
e51099f2f27702dc62885eb88048c192330ea072 codegen/lib/constants.h
4b42270f5f71ff2284982fb5e509c4bdab22c72c codegen/lib/cpu.c
88407acdfe7181a47d221253f97bbd16f562de72 codegen/lib/cpu.h
1b49716336b4a99cdb74302da0a05803873c2371 codegen/lib/data.c
fbfe4fbe89f3f44fccd4970e670927432face5a6 codegen/lib/data.h
548846911c8e0610eb611b529559a3172fac6d13 codegen/lib/external.h
ff02623fcabc97891ee76168c82a9f6c79bbda04 codegen/lib/instructions.c
347ede3443c327ca280015c24f30cd29c7b8b208 codegen/lib/instructions.h
0bdffc4d193af2be68cc7004dfd2d3f64dd73f4d codegen/lib/ppu.c
dc35769d582b685d7fc541b0ef4672fc67837b45 codegen/lib/ppu.h
4482b42e5abc750304f8a5bdeedef831b9a9c378 codegen/lib/state.c
3839132c6f37808ed588962222a69630c51cbf59 codegen/lib/state.h
EOF

echo "nathsou pin verification: PASS ($PIN; 19 blobs)"

# Xenon is big-endian. Make the palette-cache key byte-order independent.
git -C "$UPSTREAM" apply "$ROOT/patches/nathsou-ppu-endian.patch"

# C99 plain-inline public functions do not emit cross-TU definitions under
# Clang/GCC. These two functions are public API and are called from smb_main.c.
sed -i 's/^inline void update_controller1(/void update_controller1(/' "$UPSTREAM/codegen/lib/cpu.c"
sed -i 's/^inline void next_frame(/void next_frame(/' "$UPSTREAM/codegen/lib/cpu.c"

grep -q '^void update_controller1(' "$UPSTREAM/codegen/lib/cpu.c"
grep -q '^void next_frame(' "$UPSTREAM/codegen/lib/cpu.c"
grep -q 'smb360_palette_key' "$UPSTREAM/codegen/lib/ppu.c"

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
  "$ROOT/src/platform/xex/audio_xex.c"
  "$ROOT/src/platform/xex/rom_verify.c"
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
