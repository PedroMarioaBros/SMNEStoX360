#!/usr/bin/env bash
set -euo pipefail

ROOT="${OPENXECHAIN_WORKDIR:-/workspace/openxechain-cache}"
SRC="${OPENXECHAIN_SRCDIR:-/tmp/openxechain-buildscript}"
PREFIX="$ROOT/sysroot"
BUILD_COMMIT="eed1fa65bf9577fd31625764b320a90182ea9ade"
LLVM_COMMIT="890b83f6c8259a8899e182a5f7d9cf39c64131cc"
NEWLIB_COMMIT="f929633c27099d404e9cb5b2739a9c7f9b6afccc"
SYNTHXEX_COMMIT="48d1453a55468aa2f8a211db0b20edd594ef5be3"
XECORELIB_COMMIT="c65d67e071357acade681f04c46ae9719797f239"

mkdir -p "$ROOT" "$PREFIX"
git config --global url."https://github.com/".insteadOf git@github.com:

echo "=== SMB360 OpenXeChain remote builder ==="
echo "persistent root: $ROOT"
echo "ephemeral source: $SRC"
df -h "$ROOT" /tmp || true
du -sh "$ROOT" 2>/dev/null || true

# Source/build trees are intentionally ephemeral. The OpenXeChain install
# advertises the resulting sysroot as portable, so only that is persisted.
rm -rf "$SRC"
git clone https://github.com/OpenXeChain/buildscript.git "$SRC"
git -C "$SRC" checkout --detach "$BUILD_COMMIT"
git -C "$SRC" submodule sync --recursive
git -C "$SRC" submodule update --init --recursive

test "$(git -C "$SRC" rev-parse HEAD)" = "$BUILD_COMMIT"
test "$(git -C "$SRC/llvm" rev-parse HEAD)" = "$LLVM_COMMIT"
test "$(git -C "$SRC/newlib" rev-parse HEAD)" = "$NEWLIB_COMMIT"
test "$(git -C "$SRC/synthxex" rev-parse HEAD)" = "$SYNTHXEX_COMMIT"
test "$(git -C "$SRC/xecorelib" rev-parse HEAD)" = "$XECORELIB_COMMIT"

if [ -x "$PREFIX/bin/clang" ] && [ -x "$PREFIX/bin/synthxex" ]; then
  echo "CACHE_HIT: portable OpenXeChain sysroot already complete"
else
  echo "CACHE_MISS: building pinned OpenXeChain"
  export PREFIX
  export PARALLEL="${PARALLEL:-2}"
  export BUILD_TYPE=Release

  set +e
  (cd "$SRC" && bash ./build-toolchain.sh)
  rc=$?
  set -e

  if [ "$rc" -ne 0 ]; then
    echo
    echo "========== OPENXECHAIN BUILD FAILURE =========="
    echo "exit_code=$rc"
    echo "--- persistent sysroot usage ---"
    du -sh "$PREFIX" 2>/dev/null || true
    df -h "$ROOT" /tmp || true
    echo "--- tail of OpenXeChain build.log ---"
    if [ -f "$SRC/build.log" ]; then
      tail -n 500 "$SRC/build.log" || true
    else
      echo "build.log was not created"
    fi
    echo "==============================================="
    exit "$rc"
  fi
fi

test -x "$PREFIX/bin/clang"
test -x "$PREFIX/bin/lld-link"
test -x "$PREFIX/bin/synthxex"

"$PREFIX/bin/clang" --version
"$PREFIX/bin/synthxex" --version || true
printf '%s\n' "$BUILD_COMMIT" > "$ROOT/OPENXECHAIN_BUILD_COMMIT"
printf 'OpenXeChain toolchain ready at %s\n' "$PREFIX"
du -sh "$PREFIX" || true
