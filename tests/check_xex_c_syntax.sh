#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
TMP="${TMPDIR:-/tmp}/smb360-xex-syntax-$$"
trap 'rm -rf "$TMP"' EXIT
mkdir -p "$TMP/xecore"

cat > "$TMP/xecore/xboxkrnl.h" <<'EOF'
#pragma once
#include <stdint.h>
typedef int32_t NTSTATUS;
uint32_t VdGetCurrentDisplayInformation(void *info);
NTSTATUS KeDelayExecutionThread(uint32_t processor_mode, uint32_t alertable, int64_t *interval);
EOF

cat > "$TMP/cpu.h" <<'EOF'
#pragma once
#include <stdint.h>
void cpu_init(void);
void update_controller1(uint8_t state);
void next_frame(void);
EOF
cat > "$TMP/ppu.h" <<'EOF'
#pragma once
#include <stdint.h>
#define SCREEN_WIDTH 256
#define SCREEN_HEIGHT 240
extern uint8_t frame[SCREEN_WIDTH * SCREEN_HEIGHT * 3];
void ppu_render(void);
EOF
cat > "$TMP/apu.h" <<'EOF'
#pragma once
#include <stdint.h>
void apu_init(uint32_t frequency);
void apu_step_frame(void);
EOF
cat > "$TMP/code.h" <<'EOF'
#pragma once
void Start(void);
EOF
cat > "$TMP/common.h" <<'EOF'
#pragma once
int read_chr_rom(char *rom_path);
EOF

clang -std=gnu11 -Wall -Wextra -Werror -fsyntax-only \
  -I"$TMP" -I"$ROOT/src/platform/xex" \
  "$ROOT/src/platform/xex/video_fb.c"

# Host clang cannot assemble PowerPC mftb instructions, but syntax/typing can
# still be validated by replacing only the inline asm strings in a temp copy.
sed -e 's/__asm__ volatile("mftbu %0" : "=r"(hi0));/hi0 = 0;/' \
    -e 's/__asm__ volatile("mftb %0" : "=r"(lo));/lo = 0;/' \
    -e 's/__asm__ volatile("mftbu %0" : "=r"(hi1));/hi1 = 0;/' \
    "$ROOT/src/platform/xex/smb_main.c" > "$TMP/smb_main_host.c"
clang -std=gnu11 -Wall -Wextra -Werror -fsyntax-only \
  -I"$TMP" -I"$ROOT/src/platform/xex" "$TMP/smb_main_host.c"

python3 "$ROOT/tests/test_xex_fb_layout.py"
echo "XEX C backend syntax: PASS"
