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

echo "=== Verify statically identified parent-dispatch paths ==="
python3 "$ROOT/tools/analyze_unobserved_dispatchers.py" "$ROM"
echo

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
echo "Applied temporary diagnostics, data-region and fixed-slot experiments; native-only runner instrumentation is applied after reference profiling."

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

# Profile only actually selected inline-dispatch destinations in the
# interpreter-enabled reference. Unlike --miss-log, this records one target at
# the computed dispatcher boundary, rather than every subsequent interpreter
# instruction in a block. The target list stays temporary.
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

{
  cat <<'INPUT'
0 -
100 START
102 -
INPUT
  for ((FRAME=220; FRAME<8900; FRAME+=60)); do
    printf '%d RIGHT+B\n' "$FRAME"
    printf '%d RIGHT+A+B\n' "$((FRAME + 8))"
    printf '%d RIGHT+B\n' "$((FRAME + 20))"
  done
} > "$TMP/native-extended.input"

cat > "$TMP/native-run-only.input" <<'INPUT'
0 -
100 START
102 RIGHT+B
INPUT

echo "=== Instrument interpreter-enabled reference for inline dispatch ==="
python3 "$ROOT/tools/prepare_nesrecomp_dispatch_profile.py" "$TMP/nesrecomp"
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

: > "$TMP/dispatch-profile.log"
for SCENARIO in idle route jump long extended run-only; do
  INPUT_FILE=""
  case "$SCENARIO" in
    idle) FRAMES=600 ;;
    route) FRAMES=600; INPUT_FILE="$TMP/native-route.input" ;;
    jump) FRAMES=1800; INPUT_FILE="$TMP/native-jump.input" ;;
    long) FRAMES=3600; INPUT_FILE="$TMP/native-long.input" ;;
    extended) FRAMES=9000; INPUT_FILE="$TMP/native-extended.input" ;;
    run-only) FRAMES=5400; INPUT_FILE="$TMP/native-run-only.input" ;;
  esac
  ARGS=(--frames "$FRAMES")
  if [[ -n "$INPUT_FILE" ]]; then ARGS+=(--input "$INPUT_FILE"); fi
  LOG="$TMP/reference-profile-$SCENARIO.log"
  echo "=== Reference dispatch profile: scenario=$SCENARIO frames=$FRAMES ==="
  "$REFERENCE" "$ROM" "${ARGS[@]}" > "$LOG" 2>&1
  sed '/^\[INLINE_DISPATCH_PROFILE\]/d' "$LOG"
  grep '^\[INLINE_DISPATCH_PROFILE\]' "$LOG" >> "$TMP/dispatch-profile.log" || true
done
if [[ ! -s "$TMP/dispatch-profile.log" ]]; then
  echo "ERROR: reference run did not observe any \\$8E04 inline-dispatch entries." >&2
  exit 21
fi
sort -u "$TMP/dispatch-profile.log" > "$TMP/dispatch-profile-unique.log"
python3 "$ROOT/tools/verify_nesrecomp_dispatch_profile.py" "$ROM" "$TMP/dispatch-profile-unique.log" | tee "$TMP/dispatch-profile-validation.log"
echo "Unique computed inline-dispatch records: $(wc -l < "$TMP/dispatch-profile-unique.log" | tr -d ' ')"
python3 "$ROOT/tools/prepare_nesrecomp_dispatch_seeds.py" "$TMP/runtime-config.toml" "$TMP/dispatch-profile-unique.log"

# Undo profile-only edits to the temporary upstream runner before applying the
# separate no-interpreter patch. Keep the generic recompiler experiments above.
git -C "$TMP/nesrecomp" checkout --quiet "$NESRECOMP_COMMIT" -- \
  runner/cyc/cyc_run.c runner/cyc/cpu6502.h runner/cyc/cpu6502_interp.c
python3 "$ROOT/tools/prepare_nesrecomp_interpreterless.py" "$TMP/nesrecomp"
echo "=== Build native-only runner from profiled dynamic targets ==="
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

for SCENARIO in idle route jump long extended run-only; do
  INPUT_FILE=""
  case "$SCENARIO" in
    idle) FRAMES=600 ;;
    route) FRAMES=600; INPUT_FILE="$TMP/native-route.input" ;;
    jump) FRAMES=1800; INPUT_FILE="$TMP/native-jump.input" ;;
    long) FRAMES=3600; INPUT_FILE="$TMP/native-long.input" ;;
    extended) FRAMES=9000; INPUT_FILE="$TMP/native-extended.input" ;;
    run-only) FRAMES=5400; INPUT_FILE="$TMP/native-run-only.input" ;;
  esac
  ARGS=(--frames "$FRAMES")
  if [[ -n "$INPUT_FILE" ]]; then ARGS+=(--input "$INPUT_FILE"); fi
  LOG="$TMP/native-$SCENARIO.log"
  echo "=== Interpreter-free coverage: scenario=$SCENARIO frames=$FRAMES ==="
  set +e
  "$RUNTIME" "$ROM" "${ARGS[@]}" > "$LOG" 2>&1
  STATUS=$?
  set -e
  cat "$LOG"
  if [[ "$STATUS" -ne 0 ]]; then
    echo "ERROR: interpreter-free scenario '$SCENARIO' failed with exit code $STATUS. Any new NATIVE_COVERAGE_MISS is a blocking gap." >&2
    exit "$STATUS"
  fi
  echo "Native-only scenario '$SCENARIO' passed."
done
echo "Interpreter-free deterministic coverage passed using profile-observed dispatcher targets."
echo "These profile targets exist only in $TMP/runtime-config.toml; the repository config and canonical ROM are unchanged."

for SCENARIO in idle route jump long extended run-only; do
  INPUT_FILE=""
  case "$SCENARIO" in
    idle) FRAMES=600 ;;
    route) FRAMES=600; INPUT_FILE="$TMP/native-route.input" ;;
    jump) FRAMES=1800; INPUT_FILE="$TMP/native-jump.input" ;;
    long) FRAMES=3600; INPUT_FILE="$TMP/native-long.input" ;;
    extended) FRAMES=9000; INPUT_FILE="$TMP/native-extended.input" ;;
    run-only) FRAMES=5400; INPUT_FILE="$TMP/native-run-only.input" ;;
  esac
  NATIVE_HASH="$TMP/native-$SCENARIO.hash"
  REFERENCE_HASH="$TMP/reference-$SCENARIO.hash"
  NATIVE_WAV="$TMP/native-$SCENARIO.wav"
  REFERENCE_WAV="$TMP/reference-$SCENARIO.wav"
  NATIVE_LOG="$TMP/native-$SCENARIO-hash.log"
  REFERENCE_LOG="$TMP/reference-$SCENARIO.log"
  ARGS=(--frames "$FRAMES")
  if [[ -n "$INPUT_FILE" ]]; then ARGS+=(--input "$INPUT_FILE"); fi

  echo "=== Differential: scenario=$SCENARIO frames=$FRAMES ==="
  "$RUNTIME" "$ROM" "${ARGS[@]}" --hash-out "$NATIVE_HASH" --wav-out "$NATIVE_WAV" > "$NATIVE_LOG" 2>&1
  "$REFERENCE" "$ROM" "${ARGS[@]}" --hash-out "$REFERENCE_HASH" --wav-out "$REFERENCE_WAV" > "$REFERENCE_LOG" 2>&1
  cat "$NATIVE_LOG"
  cat "$REFERENCE_LOG"
  if ! cmp -s "$REFERENCE_HASH" "$NATIVE_HASH"; then
    echo "ERROR: per-frame differential mismatch in scenario '$SCENARIO'." >&2
    diff -u "$REFERENCE_HASH" "$NATIVE_HASH" | head -n 40 || true
    exit 16
  fi
  echo "Differential hashes match for every frame in scenario '$SCENARIO' ($FRAMES frames)."

  # --hash-out deliberately does not represent the exported waveform. Enable
  # audio in both runners and compare the actual 48 kHz mono PCM payload byte
  # for byte, while also checking WAV shape/sample count.
  python3 - "$NATIVE_WAV" "$REFERENCE_WAV" "$SCENARIO" <<'PY'
import hashlib
import sys
import wave

native_path, reference_path, scenario = sys.argv[1:]
def read_wav(path):
    with wave.open(path, "rb") as wf:
        params = (wf.getnchannels(), wf.getsampwidth(), wf.getframerate(), wf.getnframes(), wf.getcomptype())
        frames = wf.readframes(wf.getnframes())
    return params, frames

native_params, native_pcm = read_wav(native_path)
reference_params, reference_pcm = read_wav(reference_path)
print(f"[AUDIO_WAV] scenario={scenario} native_params={native_params} reference_params={reference_params}")
print(f"[AUDIO_PCM_SHA256] native={hashlib.sha256(native_pcm).hexdigest()} reference={hashlib.sha256(reference_pcm).hexdigest()}")
if native_params != reference_params:
    print(f"ERROR: WAV metadata differs in scenario {scenario}", file=sys.stderr)
    raise SystemExit(18)
if len(native_pcm) != len(reference_pcm):
    print(f"ERROR: PCM byte count differs in scenario {scenario}: {len(native_pcm)} vs {len(reference_pcm)}", file=sys.stderr)
    raise SystemExit(19)
if native_pcm != reference_pcm:
    first = next(i for i, (a, b) in enumerate(zip(native_pcm, reference_pcm)) if a != b)
    sample_index = first // 2
    sample_diffs = sum(native_pcm[i:i+2] != reference_pcm[i:i+2] for i in range(0, len(native_pcm), 2))
    print(f"ERROR: PCM differs in scenario {scenario}; first_byte={first} first_sample={sample_index} differing_samples={sample_diffs}", file=sys.stderr)
    raise SystemExit(20)
print(f"[AUDIO_PCM_MATCH] scenario={scenario} samples={native_params[3]} rate={native_params[2]} mono_16bit=true")
PY
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
echo "This test validates code generation and sampled native/reference parity only; it does not prove"
echo "complete function coverage, PCM equivalence on untested routes, Xbox 360 compatibility,"
echo "or a playable XEX."
