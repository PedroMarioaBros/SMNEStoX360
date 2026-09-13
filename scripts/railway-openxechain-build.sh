#!/usr/bin/env bash
set -euo pipefail

ROOT="${OPENXECHAIN_WORKDIR:-/workspace/openxechain-cache}"
SRC="$ROOT/buildscript"
PREFIX="$ROOT/sysroot"
BUILD_COMMIT="eed1fa65bf9577fd31625764b320a90182ea9ade"
LLVM_COMMIT="890b83f6c8259a8899e182a5f7d9cf39c64131cc"
NEWLIB_COMMIT="f929633c27099d404e9cb5b2739a9c7f9b6afccc"
SYNTHXEX_COMMIT="48d1453a55468aa2f8a211db0b20edd594ef5be3"
XECORELIB_COMMIT="c65d67e071357acade681f04c46ae9719797f239"

mkdir -p "$ROOT"
git config --global url."https://github.com/".insteadOf git@github.com:

if [ ! -d "$SRC/.git" ]; then
  git clone https://github.com/OpenXeChain/buildscript.git "$SRC"
fi

git -C "$SRC" fetch --prune origin
git -C "$SRC" checkout --detach "$BUILD_COMMIT"
git -C "$SRC" submodule sync --recursive
git -C "$SRC" submodule update --init --recursive

test "$(git -C "$SRC" rev-parse HEAD)" = "$BUILD_COMMIT"
test "$(git -C "$SRC/llvm" rev-parse HEAD)" = "$LLVM_COMMIT"
test "$(git -C "$SRC/newlib" rev-parse HEAD)" = "$NEWLIB_COMMIT"
test "$(git -C "$SRC/synthxex" rev-parse HEAD)" = "$SYNTHXEX_COMMIT"
test "$(git -C "$SRC/xecorelib" rev-parse HEAD)" = "$XECORELIB_COMMIT"

if [ ! -x "$PREFIX/bin/clang" ] || [ ! -x "$PREFIX/bin/synthxex" ]; then
  export PREFIX
  export PARALLEL="${PARALLEL:-$(nproc)}"
  export BUILD_TYPE=Release
  (cd "$SRC" && bash ./build-toolchain.sh)
fi

"$PREFIX/bin/clang" --version
"$PREFIX/bin/synthxex" --version
printf '%s\n' "$BUILD_COMMIT" > "$ROOT/OPENXECHAIN_BUILD_COMMIT"
printf 'OpenXeChain toolchain ready at %s\n' "$PREFIX"
