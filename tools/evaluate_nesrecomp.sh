#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
ROM="$ROOT/assets/canonical/SMB_v026.nes"
EXPECTED_SHA256="57fb4ee14288853bc0c8a5a629a103e51e87a4ebae1cc699b435060d2690b047"
NESRECOMP_REPO="https://github.com/mstan/nesrecomp.git"
NESRECOMP_COMMIT="a39d9cdbec0ac98b1430ddb53747b5ed8f8cac87"

if [[ ! -f "$ROM" ]]; then
  echo "ERROR: canonical ROM not found: $ROM" >&2
  exit 2
fi

ACTUAL_SHA256="$(sha256sum "$ROM" | awk '{print $1}')"
if [[ "$ACTUAL_SHA256" != "$EXPECTED_SHA256" ]]; then
  echo "ERROR: canonical ROM SHA-256 mismatch" >&2
  echo "Expected: $EXPECTED_SHA256" >&2
  echo "Actual:   $ACTUAL_SHA256" >&2
  exit 3
fi

TMP="$(mktemp -d "${TMPDIR:-/tmp}/smb-nesrecomp-eval.XXXXXX")"
cleanup() { rm -rf "$TMP"; }
trap cleanup EXIT

echo "=== NESRecomp evaluation ==="
echo "Repository: $NESRECOMP_REPO"
echo "Pinned commit: $NESRECOMP_COMMIT"
echo "ROM: assets/canonical/SMB_v026.nes"
echo "ROM bytes: $(wc -c < "$ROM" | tr -d ' ')"
echo "ROM SHA-256: $ACTUAL_SHA256"
echo

git init -q "$TMP/nesrecomp"
git -C "$TMP/nesrecomp" remote add origin "$NESRECOMP_REPO"
git -C "$TMP/nesrecomp" fetch --quiet --depth 1 origin "$NESRECOMP_COMMIT"
git -C "$TMP/nesrecomp" checkout --quiet --detach FETCH_HEAD

ACTUAL_COMMIT="$(git -C "$TMP/nesrecomp" rev-parse HEAD)"
if [[ "$ACTUAL_COMMIT" != "$NESRECOMP_COMMIT" ]]; then
  echo "ERROR: fetched NESRecomp commit does not match pin" >&2
  exit 4
fi

cmake -S "$TMP/nesrecomp/recompiler" -B "$TMP/build" -G Ninja \
  -DCMAKE_BUILD_TYPE=Release
cmake --build "$TMP/build" --parallel 2

mkdir -p "$TMP/run"
(
  cd "$TMP/run"
  "$TMP/build/NESRecomp" "$ROM" --cycle-accurate --output-prefix smb_v026
) 2>&1 | tee "$TMP/generation.log"

mapfile -t GENERATED_C < <(find "$TMP/run/generated" -type f -name '*.c' -print | sort)
if [[ "${#GENERATED_C[@]}" -eq 0 ]]; then
  echo "ERROR: NESRecomp completed without generating any C source files." >&2
  exit 5
fi

TOTAL_BYTES=0
TOTAL_LINES=0
for file in "${GENERATED_C[@]}"; do
  bytes="$(wc -c < "$file" | tr -d ' ')"
  lines="$(wc -l < "$file" | tr -d ' ')"
  TOTAL_BYTES=$((TOTAL_BYTES + bytes))
  TOTAL_LINES=$((TOTAL_LINES + lines))
done

echo
echo "=== Evaluation summary ==="
echo "Generated C files: ${#GENERATED_C[@]}"
echo "Generated C total bytes: $TOTAL_BYTES"
echo "Generated C total lines: $TOTAL_LINES"
echo "Generated C was kept in a temporary directory and was NOT committed."
echo "This test proves code-generation feasibility only; it does not prove"
echo "complete function coverage, no runtime fallback, Xbox 360 compatibility,"
echo "or a playable XEX."
