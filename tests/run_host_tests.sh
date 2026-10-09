#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
mkdir -p build
flags="-std=c99 -O2 -Wall -Wextra -Werror"
if [ "${SANITIZE:-0}" = 1 ]; then
 flags="$flags -g -fsanitize=address,undefined"
fi
for test in test_apu_pulse test_apu_pulse2 test_apu_triangle test_apu_noise test_apu_dmc test_canonical_nrom test_cpu6502_reset test_ppu_timing test_cpu6502 test_cpu6502_shifts test_ppu_registers test_ppu_render; do
 ${CC:-cc} $flags src/canonical/*.c "tests/$test.c" -o "build/$test"
 "build/$test"
 printf '%s: PASS\n' "$test"
done
${CC:-cc} $flags src/canonical/*.c tests/run_canonical_rom.c -o build/run_canonical_rom
