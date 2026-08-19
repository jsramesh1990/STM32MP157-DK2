#!/bin/bash
#
# STM32MP157-DK2 GPIO Project
# Clean Script
#

set -e

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

echo "=============================================="
echo " STM32MP157-DK2 GPIO Project Clean"
echo "=============================================="
echo

cd "$PROJECT_ROOT"

echo "[INFO] Cleaning application..."

if [ -f "application/Makefile" ]; then
    make -C application clean
fi

echo "[INFO] Cleaning examples..."

EXAMPLE_DIRS=(
    "examples/led"
    "examples/button"
    "examples/interrupt"
    "examples/pwm"
)

for dir in "${EXAMPLE_DIRS[@]}"; do

    if [ -f "$dir/Makefile" ]; then
        make -C "$dir" clean || true
    fi

done

echo "[INFO] Cleaning kernel driver..."

if [ -n "$KERNEL_SRC" ] && [ -f "kernel/driver/Makefile" ]; then

    make -C "$KERNEL_SRC" \
        M="$PROJECT_ROOT/kernel/driver" \
        clean || true

fi

echo "[INFO] Removing project build directory..."

rm -rf build

echo "[INFO] Removing temporary objects..."

find . \
    -type f \
    \( -name "*.o" -o -name "*.ko" -o -name "*.mod" \) \
    -delete

echo
echo "=============================================="
echo " CLEAN COMPLETED"
echo "=============================================="
