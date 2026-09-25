#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
OUT_ROOT="${SMB360_OUTPUT_ROOT:-/tmp/smb360-delivery}"
TC_ARCHIVE="/tmp/openxechain-sysroot.tar.zst"
TC_PARENT="/tmp"
TC="/tmp/openxechain-sysroot"
BUILD_WORK="/tmp/smb360-xex-build"

: "${OPENXECHAIN_URL:?OPENXECHAIN_URL is required}"

rm -rf "$OUT_ROOT" "$TC" "$BUILD_WORK"
mkdir -p "$OUT_ROOT"

echo "DOWNLOADING_OPENXECHAIN"
wget -q --show-progress --tries=4 --timeout=60 "$OPENXECHAIN_URL" -O "$TC_ARCHIVE"

echo "EXTRACTING_OPENXECHAIN"
tar -I zstd -xf "$TC_ARCHIVE" -C "$TC_PARENT"

test -x "$TC/bin/clang"
test -x "$TC/bin/lld-link"
test -x "$TC/bin/synthxex"

echo "BUILDING_SMB360"
chmod +x "$ROOT/scripts/build-smb-xex.sh"
OPENXECHAIN_PREFIX="$TC" \
SMB360_XEX_WORK="$BUILD_WORK" \
"$ROOT/scripts/build-smb-xex.sh"

XEX="$BUILD_WORK/out/default.xex"
test -s "$XEX"
test "$(dd if="$XEX" bs=1 count=4 2>/dev/null)" = "XEX2"

mkdir -p "$OUT_ROOT/SMB360"
cp "$XEX" "$OUT_ROOT/SMB360/default.xex"
cp "$BUILD_WORK/out/SHA256SUMS.txt" "$OUT_ROOT/SMB360/SHA256SUMS.txt"
cp "$BUILD_WORK/out/BUILD_MANIFEST.txt" "$OUT_ROOT/SMB360/BUILD_MANIFEST.txt"

cat > "$OUT_ROOT/SMB360/LEIA-ME.txt" <<'EOF'
SMB360 - build nativo para Xbox 360.
A ROM nao faz parte deste pacote.
Use somente a ROM legal do usuario esperada pelo projeto, nomeada smb.nes,
ao lado de default.xex.
EOF

(
  cd "$OUT_ROOT"
  zip -r SMB360-XEX.zip SMB360 >/dev/null
)
sha256sum "$OUT_ROOT/SMB360-XEX.zip" > "$OUT_ROOT/SMB360-XEX.zip.sha256"
echo "BUILD_READY" > "$OUT_ROOT/status.txt"

echo "SMB360_XEX_READY"
ls -lh "$OUT_ROOT/SMB360/default.xex" "$OUT_ROOT/SMB360-XEX.zip"
exec python3 -m http.server "${PORT:-8080}" --bind 0.0.0.0 --directory "$OUT_ROOT"
