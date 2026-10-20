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
python3 "$ROOT/tools/prepare_nesrecomp_fixed_slot_crossing.py" "$TMP/nesrecomp"
python3 "$ROOT/tools/prepare_nesrecomp_inline_jmp.py" "$TMP/nesrecomp"
python3 "$ROOT/tools/prepare_nesrecomp_interpreterless.py" "$TMP/nesrecomp"
echo "Applied temporary diagnostics, data-region, fixed-slot, and interpreter-free experiments; generated C remains private to temp."

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

# Coverage-guided static seeding: on a fail-closed miss, add only the observed
# ROM entry to a TEMPORARY copy of the config, regenerate, rebuild without the
# interpreter, and replay the deterministic routes. This removes manual
# one-target-at-a-time edits while keeping the canonical ROM/config untouched.
cp "$ROOT/tools/nesrecomp_smb_v026_data_region_experiment.toml" "$TMP/runtime-config.toml"
cat > "$TMP/native-route.input" <<'INPUT'
0 -
100 START
102 -
220 RIGHT+A
270 RIGHT
360 -
INPUT
cat > "$TMP/native-jump.input" <<'INPUT'
0 -
100 START
102 -
220 RIGHT+A+B
280 RIGHT
360 RIGHT+A
420 RIGHT
460 -
520 A
521 -
560 RIGHT+A+B
700 RIGHT
760 -
INPUT
cat > "$TMP/native-long.input" <<'INPUT'
0 -
100 START
102 -
220 RIGHT+A+B
280 RIGHT
360 RIGHT+A+B
440 RIGHT+B
580 -
700 RIGHT+A
800 RIGHT
900 LEFT
930 -
1000 RIGHT+A+B
1150 RIGHT
1300 B
1400 RIGHT+A
1600 RIGHT+B
1900 RIGHT
2100 LEFT
2150 -
2300 RIGHT+A+B
2600 RIGHT
2900 A
2901 -
3100 RIGHT+A
3350 RIGHT+B
3500 -
INPUT

MAX_NATIVE_AUTO_SEEDS=24
AUTO_NATIVE_SEEDS=0
ATTEMPT=1
while true; do
  echo "=== Interpreter-free build attempt $ATTEMPT (auto-seeded targets: $AUTO_NATIVE_SEEDS) ==="
  rm -rf "$TMP/build-native-only"
  cmake -S "$TMP/nesrecomp/runner/cyc/project" -B "$TMP/build-native-only" -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DNESRECOMP_ROM="$ROM" \
    -DNESRECOMP_GAME_CONFIG="$TMP/runtime-config.toml" \
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

  NEEDS_REBUILD=0
  for SCENARIO in idle route jump long; do
    INPUT_FILE=""
    case "$SCENARIO" in
      idle) FRAMES=600; LOG="$TMP/native-idle.log" ;;
      route) FRAMES=600; INPUT_FILE="$TMP/native-route.input"; LOG="$TMP/native-route.log" ;;
      jump) FRAMES=1800; INPUT_FILE="$TMP/native-jump.input"; LOG="$TMP/native-jump.log" ;;
      long) FRAMES=3600; INPUT_FILE="$TMP/native-long.input"; LOG="$TMP/native-long.log" ;;
    esac
    ARGS=(--frames "$FRAMES")
    if [[ -n "$INPUT_FILE" ]]; then ARGS+=(--input "$INPUT_FILE"); fi
    if [[ "$SCENARIO" == "idle" ]]; then ARGS+=(--miss-log "$TMP/native-idle-miss.log"); fi
    echo "=== Interpreter-free runtime: scenario=$SCENARIO frames=$FRAMES ==="
    set +e
    "$RUNTIME" "$ROM" "${ARGS[@]}" > "$LOG" 2>&1
    STATUS=$?
    set -e
    cat "$LOG"
    if [[ "$STATUS" -eq 0 ]]; then
      echo "Native-only scenario '$SCENARIO' passed."
      continue
    fi
    if [[ "$STATUS" -ne 86 ]]; then
      echo "ERROR: scenario '$SCENARIO' failed with exit code $STATUS, not a coverage miss." >&2
      exit "$STATUS"
    fi

    MISSED_PC="$(sed -n 's/.*pc=\$\([0-9A-Fa-f]\{4\}\).*/\1/p' "$LOG" | head -n 1)"
    if [[ -z "$MISSED_PC" ]]; then
      echo "ERROR: exit code 86 without a parseable [NATIVE_COVERAGE_MISS] PC." >&2
      exit 12
    fi
    if [[ "$AUTO_NATIVE_SEEDS" -ge "$MAX_NATIVE_AUTO_SEEDS" ]]; then
      echo "ERROR: hit the automatic static-seed cap ($MAX_NATIVE_AUTO_SEEDS) at PC $MISSED_PC." >&2
      exit 13
    fi
    python3 "$ROOT/tools/prepare_nesrecomp_runtime_seed.py" "$TMP/runtime-config.toml" "$MISSED_PC"
    AUTO_NATIVE_SEEDS=$((AUTO_NATIVE_SEEDS + 1))
    ATTEMPT=$((ATTEMPT + 1))
    NEEDS_REBUILD=1
    break
  done

  if [[ "$NEEDS_REBUILD" -eq 1 ]]; then
    continue
  fi
  break
done

echo "Interpreter-free deterministic route coverage passed after $AUTO_NATIVE_SEEDS automatic ROM-target additions."
echo "The auto-seeds exist only in $TMP/runtime-config.toml and are not committed."
# Differential gate: compare the seeded interpreter-free runner against the
# ordinary runtime using the original project experiment config. The reference
# runtime keeps the interpreter enabled, so it can execute indirect targets
# that are not among the original static seeds. Both receive identical ROM and
# deterministic inputs; generated C stays under TMP.
echo "=== Build interpreter-enabled reference runner (original experimental config) ==="
git -C "$TMP/nesrecomp" checkout --quiet HEAD -- \
  runner/cyc/cyc.cmake runner/cyc/cyc_run.c runner/cyc/cpu6502.h
rm -f "$TMP/nesrecomp/runner/cyc/cpu6502_op_length.c"
cmake -S "$TMP/nesrecomp/runner/cyc/project" -B "$TMP/build-reference" -G Ninja \
  -DCMAKE_BUILD_TYPE=Release \
  -DNESRECOMP_ROM="$ROM" \
  -DNESRECOMP_GAME_CONFIG="$ROOT/tools/nesrecomp_smb_v026_data_region_experiment.toml" \
  -DNESRECOMP_HOST_COMPILER="$TMP/build/NESRecomp" \
  -DNESRECOMP_HEADLESS=ON
cmake --build "$TMP/build-reference" --parallel 2
REFERENCE="$TMP/build-reference/nes_game"
if [[ ! -x "$REFERENCE" ]]; then
  echo "ERROR: interpreter-enabled reference runner was not produced." >&2
  exit 15
fi

for SCENARIO in idle route jump long; do
  INPUT_FILE=""
  case "$SCENARIO" in
    idle) FRAMES=600 ;;
    route) FRAMES=600; INPUT_FILE="$TMP/native-route.input" ;;
    jump) FRAMES=1800; INPUT_FILE="$TMP/native-jump.input" ;;
    long) FRAMES=3600; INPUT_FILE="$TMP/native-long.input" ;;
  esac
  NATIVE_HASH="$TMP/native-$SCENARIO.hash"
  REFERENCE_HASH="$TMP/reference-$SCENARIO.hash"
  NATIVE_LOG="$TMP/native-$SCENARIO-hash.log"
  REFERENCE_LOG="$TMP/reference-$SCENARIO.log"
  ARGS=(--frames "$FRAMES")
  if [[ -n "$INPUT_FILE" ]]; then ARGS+=(--input "$INPUT_FILE"); fi

  echo "=== Differential: scenario=$SCENARIO frames=$FRAMES ==="
  "$RUNTIME" "$ROM" "${ARGS[@]}" --hash-out "$NATIVE_HASH" > "$NATIVE_LOG" 2>&1
  "$REFERENCE" "$ROM" "${ARGS[@]}" --hash-out "$REFERENCE_HASH" > "$REFERENCE_LOG" 2>&1
  cat "$NATIVE_LOG"
  cat "$REFERENCE_LOG"
  if ! cmp -s "$REFERENCE_HASH" "$NATIVE_HASH"; then
    echo "ERROR: per-frame differential mismatch in scenario '$SCENARIO'." >&2
    diff -u "$REFERENCE_HASH" "$NATIVE_HASH" | head -n 40 || true
    exit 16
  fi
  echo "Differential hashes match for every frame in scenario '$SCENARIO' ($FRAMES frames)."
done
echo "Interpreter-free runner matched the interpreter-enabled reference across all four deterministic scenarios."

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
