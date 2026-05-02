#!/usr/bin/env bash
# build.sh — compile RAFAELIA enterprise para ARM64, x86-64, ARM32 (Termux)
set -euo pipefail

echo "=== RAFAELIA Enterprise Build ==="

CFLAGS="-O2 -nostdlib -ffreestanding -fno-stack-protector \
        -fno-asynchronous-unwind-tables -fno-plt \
        -Wall -Wextra -Wno-unused-function \
        -I."
# ── ARM32 (Termux correto) ───────────────────────────────────────────────
if [[ "$(uname -m)" == "armv7l" ]]; then
    echo "[ARM32] Termux build (EABI softfp compatível)..."

    gcc $CFLAGS \
        -march=armv7-a \
        -mfloat-abi=softfp \
        -mfpu=neon \
        -fno-pic \
        -Wl,--build-id=none \
        -Wl,-z,norelro \
        -e _start \
        -o raf_enterprise_a32 \
        raf_main.c

    echo "[ARM32] OK -> raf_enterprise_a32"
    file raf_enterprise_a32
fi
# ── ARM64 (nativo) ────────────────────────────────────────────────────────
if [[ "$(uname -m)" == "aarch64" ]]; then
    echo "[ARM64] gcc -march=armv8.2-a+crc+crypto ..."
    gcc $CFLAGS \
        -march=armv8.2-a+crc+crypto \
        -mtune=cortex-a78 \
        -static \
        -e _start \
        -o raf_enterprise_a64 \
        raf_main.c
    echo "[ARM64] OK -> raf_enterprise_a64"
    size raf_enterprise_a64
fi

# ── x86-64 ───────────────────────────────────────────────────────────────
if [[ "$(uname -m)" == "x86_64" ]]; then
    echo "[x86-64] gcc -march=native ..."
    gcc $CFLAGS \
        -march=native \
        -static \
        -e _start \
        -o raf_enterprise_x64 \
        raf_main.c
    echo "[x86-64] OK -> raf_enterprise_x64"
    size raf_enterprise_x64
fi

# ── ARM32 nativo (Termux) - SEM -mfloat-abi=hard ─────────────────────────
if [[ "$(uname -m)" == armv7l || "$(uname -m)" == armv8l || "$(uname -m)" == arm* ]]; then
    echo "[ARM32] compilação nativa para Termux (softfp, timer via clock_gettime) ..."
    gcc $CFLAGS \
        -march=armv7-a \
        -static \
        -e _start \
        -o raf_enterprise_a32 \
        raf_main.c
    echo "[ARM32] OK -> raf_enterprise_a32"
fi

# ── ARM32 cross (se toolchain disponível) ────────────────────────────────
if command -v arm-linux-gnueabihf-gcc &>/dev/null; then
    echo "[ARM32] cross-compile (hard-float) ..."
    arm-linux-gnueabihf-gcc $CFLAGS \
        -march=armv7-a+fp \
        -mfpu=neon-vfpv4 \
        -mfloat-abi=hard \
        -static \
        -e _start \
        -o raf_enterprise_a32_hf \
        raf_main.c
    echo "[ARM32] OK -> raf_enterprise_a32_hf"
fi

echo "=== Build complete ==="
for b in raf_enterprise_*; do
    [[ -f "$b" ]] && strip --strip-all "$b" && echo "stripped: $(du -sh $b)"
done
