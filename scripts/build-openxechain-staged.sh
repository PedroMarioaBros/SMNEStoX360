#!/usr/bin/env bash
set -euo pipefail

# Resumable OpenXeChain builder for constrained cloud executors.
# Usage:
#   WORK=/workspace/ox PREFIX=/workspace/ox/sysroot PARALLEL=2 \
#     bash scripts/build-openxechain-staged.sh [all|llvm|xecorelib|newlib|compiler-rt|synthxex]
#
# Source tree layout under $WORK/src:
#   buildscript/  llvm/  newlib/  synthxex/  xecorelib/
# Each source directory must be checked out at the pinned commit below.

STAGE="${1:-all}"
WORK="${WORK:-$PWD/.openxechain}"
SRC="$WORK/src"
BUILD="$WORK/build"
PREFIX="${PREFIX:-$WORK/sysroot}"
PARALLEL="${PARALLEL:-$(nproc)}"
BUILD_TYPE="${BUILD_TYPE:-Release}"

LLVM_COMMIT=890b83f6c8259a8899e182a5f7d9cf39c64131cc
NEWLIB_COMMIT=f929633c27099d404e9cb5b2739a9c7f9b6afccc
SYNTHXEX_COMMIT=48d1453a55468aa2f8a211db0b20edd594ef5be3
XECORELIB_COMMIT=c65d67e071357acade681f04c46ae9719797f239

mkdir -p "$SRC" "$BUILD" "$PREFIX" "$WORK/stamps" "$WORK/logs"

need_repo() {
  local dir="$1" expected="$2"
  test -d "$dir/.git" || { echo "missing source repo: $dir" >&2; exit 2; }
  local actual
  actual="$(git -C "$dir" rev-parse HEAD)"
  test "$actual" = "$expected" || {
    echo "pin mismatch: $dir expected=$expected actual=$actual" >&2
    exit 3
  }
}

verify_sources() {
  need_repo "$SRC/llvm" "$LLVM_COMMIT"
  need_repo "$SRC/newlib" "$NEWLIB_COMMIT"
  need_repo "$SRC/synthxex" "$SYNTHXEX_COMMIT"
  need_repo "$SRC/xecorelib" "$XECORELIB_COMMIT"
}

run_logged() {
  local name="$1"; shift
  echo "=== $name $(date -u +%FT%TZ) ===" | tee "$WORK/logs/$name.log"
  "$@" 2>&1 | tee -a "$WORK/logs/$name.log"
}

build_llvm() {
  test -f "$WORK/stamps/llvm.ok" && return 0
  rm -rf "$BUILD/llvm"
  cmake -S "$SRC/llvm/llvm" -B "$BUILD/llvm" -G Ninja \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DCMAKE_C_COMPILER=clang \
    -DCMAKE_CXX_COMPILER=clang++ \
    -DCMAKE_INSTALL_PREFIX="$PREFIX" \
    -DLLVM_ENABLE_PROJECTS='lld;clang' \
    -DLLVM_TARGETS_TO_BUILD=PowerPC \
    -DLLVM_DEFAULT_TARGET_TRIPLE=ppc32-xbox360 \
    -DLLVM_INSTALL_BINUTILS_SYMLINKS=true \
    -DLLVM_INSTALL_CCTOOLS_SYMLINKS=true \
    -DLLVM_INSTALL_TOOLCHAIN_ONLY=true
  run_logged llvm-build cmake --build "$BUILD/llvm" -j "$PARALLEL"
  run_logged llvm-install cmake --install "$BUILD/llvm"
  test -x "$PREFIX/bin/clang"
  test -x "$PREFIX/bin/lld-link"
  cat > "$PREFIX/bin/clang.cfg" <<'EOF'
-Wno-main-return-type
--sysroot=<CFGDIR>/..
--rtlib=compiler-rt
-fdeclspec
-mlongcall
EOF
  cp "$PREFIX/bin/clang.cfg" "$PREFIX/bin/clang++.cfg"
  touch "$WORK/stamps/llvm.ok"
  tar -C "$WORK" -czf "$WORK/openxechain-stage-llvm.tar.gz" sysroot stamps/llvm.ok
}

build_xecorelib() {
  test -f "$WORK/stamps/xecorelib.ok" && return 0
  build_llvm
  run_logged xecorelib-install env PREFIX="$PREFIX" bash "$SRC/xecorelib/install.sh"
  rm -rf "$BUILD/xecorelibtmp"
  run_logged xecorelib-temp env BINDIR="$PREFIX/bin" PREFIX="$BUILD/xecorelibtmp" bash "$SRC/xecorelib/install.sh"
  touch "$WORK/stamps/xecorelib.ok"
  tar -C "$WORK" -czf "$WORK/openxechain-stage-xecorelib.tar.gz" sysroot stamps/llvm.ok stamps/xecorelib.ok
}

build_newlib() {
  test -f "$WORK/stamps/newlib.ok" && return 0
  build_xecorelib
  rm -rf "$BUILD/newlib"
  mkdir -p "$BUILD/newlib"
  (
    cd "$BUILD/newlib"
    env \
      CC="$PREFIX/bin/clang -nostdlib -I$BUILD/xecorelibtmp/include" \
      CPP="$PREFIX/bin/clang-cpp" \
      LD="$PREFIX/bin/lld-link" \
      AR="$PREFIX/bin/llvm-ar" \
      AS="$PREFIX/bin/llvm-as" \
      STRIP="$PREFIX/bin/llvm-strip" \
      RANLIB="$PREFIX/bin/llvm-ranlib" \
      "$SRC/newlib/newlib/configure" \
      --prefix="$PREFIX" \
      --host=ppc-xbox360 \
      --target=ppc-xbox360 \
      --enable-newlib-supplied-syscalls=yes \
      --enable-newlib-mb \
      --enable-newlib-iconv 2>&1 | tee "$WORK/logs/newlib-configure.log"
    make -j"$PARALLEL" 2>&1 | tee "$WORK/logs/newlib-build.log"
    make install 2>&1 | tee "$WORK/logs/newlib-install.log"
  )
  cat >> "$PREFIX/bin/clang.cfg" <<'EOF'
-isystem <CFGDIR>/../ppc-xbox360/include
-isystem <CFGDIR>/../include
-Wl,/libpath:<CFGDIR>/../ppc-xbox360/lib,/libpath:<CFGDIR>/../lib
-Wl,/defaultlib:xecorelib.a,/defaultlib:libc.a
EOF
  cp "$PREFIX/bin/clang.cfg" "$PREFIX/bin/clang++.cfg"
  touch "$WORK/stamps/newlib.ok"
  tar -C "$WORK" -czf "$WORK/openxechain-stage-newlib.tar.gz" sysroot stamps/llvm.ok stamps/xecorelib.ok stamps/newlib.ok
}

build_compiler_rt() {
  test -f "$WORK/stamps/compiler-rt.ok" && return 0
  build_newlib
  rm -rf "$BUILD/compiler-rt"
  cmake -S "$SRC/llvm/compiler-rt" -B "$BUILD/compiler-rt" -G Ninja \
    -DCMAKE_INSTALL_PREFIX="$PREFIX" \
    -DCMAKE_SYSTEM_NAME=Generic \
    -DCMAKE_CROSSCOMPILING=true \
    -DCMAKE_C_COMPILER="$PREFIX/bin/clang" \
    -DCMAKE_CXX_COMPILER="$PREFIX/bin/clang++" \
    -DCMAKE_AR="$PREFIX/bin/llvm-ar" \
    -DCMAKE_LINKER="$PREFIX/bin/lld-link" \
    -DCMAKE_RANLIB="$PREFIX/bin/llvm-ranlib" \
    -DCMAKE_SYSROOT="$PREFIX" \
    -DCMAKE_C_COMPILER_WORKS=true \
    -DCMAKE_CXX_COMPILER_WORKS=true \
    -DCMAKE_C_COMPILER_TARGET=ppc32-xbox360 \
    -DCOMPILER_RT_BUILD_BUILTINS=true \
    -DCOMPILER_RT_DEFAULT_TARGET_ONLY=true \
    -DCOMPILER_RT_BUILD_SANITIZERS=false \
    -DCOMPILER_RT_BUILD_XRAY=false \
    -DCOMPILER_RT_BUILD_LIBFUZZER=false \
    -DCOMPILER_RT_BUILD_PROFILE=false \
    -DCOMPILER_RT_STANDALONE_BUILD=true \
    -DCOMPILER_RT_BUILTINS_ENABLE_PIC=false \
    -DCOMPILER_RT_BAREMETAL_BUILD=true
  run_logged compiler-rt-build cmake --build "$BUILD/compiler-rt" -j "$PARALLEL"
  run_logged compiler-rt-install cmake --install "$BUILD/compiler-rt"
  cat >> "$PREFIX/bin/clang.cfg" <<'EOF'
-Wl,/libpath:<CFGDIR>/../lib/generic
-Wl,/defaultlib:libclang_rt.builtins-powerpc.a
EOF
  cp "$PREFIX/bin/clang.cfg" "$PREFIX/bin/clang++.cfg"
  touch "$WORK/stamps/compiler-rt.ok"
}

build_synthxex() {
  test -f "$WORK/stamps/synthxex.ok" && return 0
  build_compiler_rt
  rm -rf "$BUILD/synthxex"
  cmake -S "$SRC/synthxex" -B "$BUILD/synthxex" -G Ninja \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DCMAKE_C_COMPILER=clang \
    -DCMAKE_INSTALL_PREFIX="$PREFIX"
  run_logged synthxex-build cmake --build "$BUILD/synthxex" -j "$PARALLEL"
  run_logged synthxex-install cmake --install "$BUILD/synthxex"
  test -x "$PREFIX/bin/synthxex"
  touch "$WORK/stamps/synthxex.ok"
  tar -C "$WORK" -czf "$WORK/openxechain-complete.tar.gz" sysroot stamps
}

verify_sources
case "$STAGE" in
  llvm) build_llvm ;;
  xecorelib) build_xecorelib ;;
  newlib) build_newlib ;;
  compiler-rt) build_compiler_rt ;;
  synthxex|all) build_synthxex ;;
  *) echo "unknown stage: $STAGE" >&2; exit 64 ;;
esac

echo "OpenXeChain stage '$STAGE' PASS"
