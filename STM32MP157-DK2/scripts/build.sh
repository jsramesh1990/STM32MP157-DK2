#!/bin/bash
#
# STM32MP157-DK2 GPIO Project
# Build Script
#

set -e

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

echo "=============================================="
echo " STM32MP157-DK2 GPIO Project Build"
echo "=============================================="
echo

cd "$PROJECT_ROOT"

echo "[INFO] Project root:"
echo "       $PROJECT_ROOT"
echo

# --------------------------------------------------
# Check required tools
# --------------------------------------------------

echo "[1/6] Checking build tools..."

for tool in gcc make; do
    if ! command -v "$tool" >/dev/null 2>&1; then
        echo "[ERROR] $tool not found."
        exit 1
    fi
done

echo "[OK] Build tools available."
echo

# --------------------------------------------------
# Create build directory
# --------------------------------------------------

echo "[2/6] Creating build directory..."

mkdir -p build
mkdir -p build/bin
mkdir -p build/lib
mkdir -p build/examples
mkdir -p build/driver

echo "[OK] Build directories created."
echo

# --------------------------------------------------
# Build application
# --------------------------------------------------

echo "[3/6] Building GPIO application..."

if [ -f "application/Makefile" ]; then
    make -C application clean
    make -C application
else
    echo "[WARNING] application/Makefile not found."
    echo "[INFO] Building application sources manually..."

    gcc -Wall -Wextra -O2 \
        -Iapplication/include \
        -c application/src/gpio-common.c \
        -o build/gpio-common.o

    gcc -Wall -Wextra -O2 \
        -Iapplication/include \
        -c application/src/gpio-sysfs.c \
        -o build/gpio-sysfs.o

    gcc -Wall -Wextra -O2 \
        -Iapplication/include \
        -c application/src/gpio-libgpiod.c \
        -o build/gpio-libgpiod.o
fi

echo "[OK] GPIO application build completed."
echo

# --------------------------------------------------
# Build examples
# --------------------------------------------------

echo "[4/6] Building examples..."

EXAMPLE_DIRS=(
    "examples/led"
    "examples/button"
    "examples/interrupt"
    "examples/pwm"
)

for dir in "${EXAMPLE_DIRS[@]}"; do

    if [ -f "$dir/Makefile" ]; then
        echo "[INFO] Building $dir..."
        make -C "$dir"
    else
        echo "[WARNING] Makefile missing in $dir"
    fi

done

echo "[OK] Examples built."
echo

# --------------------------------------------------
# Build kernel driver
# --------------------------------------------------

echo "[5/6] Checking kernel driver..."

if [ -f "kernel/driver/Makefile" ]; then

    echo "[INFO] Kernel driver source found."

    if [ -n "$KERNEL_SRC" ]; then
        echo "[INFO] Kernel source: $KERNEL_SRC"

        make -C "$KERNEL_SRC" \
            M="$PROJECT_ROOT/kernel/driver" \
            modules

    else
        echo "[WARNING] KERNEL_SRC is not set."
        echo
        echo "To build the kernel module:"
        echo
        echo "export KERNEL_SRC=/path/to/linux"
        echo "./scripts/build.sh"
        echo
    fi

else
    echo "[WARNING] Kernel driver Makefile not found."
fi

echo

# --------------------------------------------------
# Build summary
# --------------------------------------------------

echo "[6/6] Build summary"
echo

echo "Project:"
echo "  STM32MP157-DK2"

echo
echo "Components:"
echo "  - GPIO common layer"
echo "  - GPIO Sysfs interface"
echo "  - GPIO libgpiod interface"
echo "  - LED examples"
echo "  - Button examples"
echo "  - Interrupt examples"
echo "  - PWM examples"
echo "  - Virtual GPIO kernel driver"

echo
echo "=============================================="
echo " BUILD COMPLETED SUCCESSFULLY"
echo "=============================================="
