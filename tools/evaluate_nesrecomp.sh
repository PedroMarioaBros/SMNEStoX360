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

# Apply an evaluation-only instrumentation patch to the temporary upstream
# checkout. The upstream commit remains pinned and the patch is not vendored.
git -C "$TMP/nesrecomp" apply "$ROOT/tools/nesrecomp-tail-diagnostic.patch"
git -C "$TMP/nesrecomp" apply "$ROOT/tools/nesrecomp-data-region-vector.patch"
echo "Applied temporary diagnostic and data-region experiment patches; generated C remains private to temp."

cmake -S "$TMP/nesrecomp/recompiler" -B "$TMP/build" -G Ninja \
  -DCMAKE_BUILD_TYPE=Release
cmake --build "$TMP/build" --parallel 2

mkdir -p "$TMP/run"
(
  cd "$TMP/run"
  "$TMP/build/NESRecomp" "$ROM" --cycle-accurate --output-prefix smb_v026
) 2>&1 | tee "$TMP/generation.log"

TAIL_DIAG_COUNT="$(grep -c '^\[TAIL_DIAGNOSTIC\]' "$TMP/generation.log" || true)"
SUMMARY_TAIL_COUNT="$(sed -n 's/.*instructions compiled, \([0-9][0-9]*\) left to the interpreter.*/\1/p' "$TMP/generation.log" | tail -n 1)"
if [[ -z "$SUMMARY_TAIL_COUNT" ]]; then
  SUMMARY_TAIL_COUNT=0
fi
if [[ "$TAIL_DIAG_COUNT" != "$SUMMARY_TAIL_COUNT" ]]; then
  echo "ERROR: diagnostic count ($TAIL_DIAG_COUNT) does not match generator tail count ($SUMMARY_TAIL_COUNT)." >&2
  exit 6
fi
echo "Tail diagnostic entries: $TAIL_DIAG_COUNT"

# Controlled experiment: honor an explicit data_region annotation when deciding
# whether to seed an interrupt-vector target as code. This is a temporary source
# patch plus a project-specific TOML hypothesis, not a claim about runtime safety.
mkdir -p "$TMP/run-data-region"
(
  cd "$TMP/run-data-region"
  "$TMP/build/NESRecomp" "$ROM" --cycle-accurate \
    --game "$ROOT/tools/nesrecomp_smb_v026_data_region_experiment.toml" \
    --output-prefix smb_v026_data_region
) 2>&1 | tee "$TMP/generation-data-region.log"

FILTER_TAIL_DIAG_COUNT="$(grep -c '^\[TAIL_DIAGNOSTIC\]' "$TMP/generation-data-region.log" || true)"
FILTER_SUMMARY_TAIL_COUNT="$(sed -n 's/.*instructions compiled, \([0-9][0-9]*\) left to the interpreter.*/\1/p' "$TMP/generation-data-region.log" | tail -n 1)"
if [[ -z "$FILTER_SUMMARY_TAIL_COUNT" ]]; then
  FILTER_SUMMARY_TAIL_COUNT=0
fi
if [[ "$FILTER_TAIL_DIAG_COUNT" != "$FILTER_SUMMARY_TAIL_COUNT" ]]; then
  echo "ERROR: configured diagnostic count ($FILTER_TAIL_DIAG_COUNT) does not match tail count ($FILTER_SUMMARY_TAIL_COUNT)." >&2
  exit 7
fi
if [[ "$FILTER_SUMMARY_TAIL_COUNT" != "0" ]]; then
  echo "ERROR: declared data_region did not eliminate the false IRQ-vector tail." >&2
  exit 8
fi
echo "Data-region experiment: [VECTOR_DATA_REGION] skip was applied; fallback-tail count=0."

# Temporary runtime adaptation: exclude the interpreter translation unit and
# make every uncovered dispatch fail closed. This is not a product port; it is
# a coverage proof attempt for deterministic host scenarios only.
python3 "$ROOT/tools/prepare_nesrecomp_interpreterless.py" "$TMP/nesrecomp"

cmake -S "$TMP/nesrecomp/runner/cyc/project" -B "$TMP/build-native-only" -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DNESRECOMP_ROM="$ROM" \
  -DNESRECOMP_GAME_CONFIG="$ROOT/tools/nesrecomp_smb_v026_data_region_experiment.toml" \
  -DNESRECOMP_HOST_COMPILER="$TMP/build/NESRecomp" \
  -DNESRECOMP_HEADLESS=ON \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build "$TMP/build-native-only" --parallel 2

RUNTIME="$TMP/build-native-only/nes_game"
if [[ ! -x "$RUNTIME" ]]; then
  echo "ERROR: interpreter-free headless runner was not produced." >&2
  exit 9
fi
if grep -Fq "cpu6502_interp.c" "$TMP/build-native-only/build.ninja"; then
  echo "ERROR: interpreter source is still listed in the runtime build." >&2
  exit 10
fi
nm "$RUNTIME" > "$TMP/runtime-symbols.txt"
if grep -q 'cpu_interp_step' "$TMP/runtime-symbols.txt"; then
  echo "ERROR: cpu_interp_step is linked into the interpreter-free runtime." >&2
  exit 11
fi
echo "Runtime inspection: cpu6502_interp.c absent from build.ninja; cpu_interp_step absent from executable symbols."

cat > "$TMP/native-route.input" <<'INPUT'
0 -
100 START
102 -
220 RIGHT+A
270 RIGHT
360 -
INPUT

echo "=== Interpreter-free runtime: 600 idle/title frames ==="
"$RUNTIME" "$ROM" --frames 600 --miss-log "$TMP/native-idle-miss.log" \
  2>&1 | tee "$TMP/native-idle.log"
echo "=== Interpreter-free runtime: 600 scripted-input frames ==="
"$RUNTIME" "$ROM" --frames 600 --input "$TMP/native-route.input" \
  --miss-log "$TMP/native-route-miss.log" 2>&1 | tee "$TMP/native-route.log"
echo "Interpreter-free host runtime passed both 600-frame scenarios without an uncovered-PC trap."

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
