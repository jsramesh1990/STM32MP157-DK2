#!/bin/bash
#
# STM32MP157-DK2 GPIO Project
# Deploy Script
#

set -e

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

# Default target configuration
TARGET_USER="${TARGET_USER:-root}"
TARGET_HOST="${TARGET_HOST:-192.168.1.100}"
TARGET_DIR="${TARGET_DIR:-/opt/stm32mp157-gpio}"

echo "=============================================="
echo " STM32MP157-DK2 GPIO Project Deployment"
echo "=============================================="
echo

echo "[INFO] Target:"
echo "       User : $TARGET_USER"
echo "       Host : $TARGET_HOST"
echo "       Path : $TARGET_DIR"
echo

# --------------------------------------------------
# Check SSH
# --------------------------------------------------

if ! command -v ssh >/dev/null 2>&1; then
    echo "[ERROR] ssh command not found."
    exit 1
fi

if ! command -v scp >/dev/null 2>&1; then
    echo "[ERROR] scp command not found."
    exit 1
fi

# --------------------------------------------------
# Check connection
# --------------------------------------------------

echo "[1/5] Checking target connection..."

if ! ssh "${TARGET_USER}@${TARGET_HOST}" "echo STM32MP157-DK2 connected" >/dev/null 2>&1; then
    echo "[ERROR] Cannot connect to target."
    echo
    echo "Example:"
    echo
    echo "TARGET_HOST=192.168.1.50 ./scripts/deploy.sh"
    echo
    exit 1
fi

echo "[OK] Target reachable."
echo

# --------------------------------------------------
# Create target directory
# --------------------------------------------------

echo "[2/5] Creating target directory..."

ssh "${TARGET_USER}@${TARGET_HOST}" \
    "mkdir -p $TARGET_DIR/bin $TARGET_DIR/config $TARGET_DIR/scripts"

echo "[OK] Target directory created."
echo

# --------------------------------------------------
# Deploy application
# --------------------------------------------------

echo "[3/5] Deploying applications..."

if [ -d "$PROJECT_ROOT/application/bin" ]; then

    scp -r \
        "$PROJECT_ROOT/application/bin/"* \
        "${TARGET_USER}@${TARGET_HOST}:${TARGET_DIR}/bin/" \
        2>/dev/null || true

fi

# --------------------------------------------------
# Deploy examples
# --------------------------------------------------

echo "[4/5] Deploying examples..."

for dir in led button interrupt pwm; do

    if [ -d "$PROJECT_ROOT/examples/$dir" ]; then

        ssh "${TARGET_USER}@${TARGET_HOST}" \
            "mkdir -p $TARGET_DIR/bin/$dir"

        find "$PROJECT_ROOT/examples/$dir" \
            -maxdepth 1 \
            -type f \
            -perm -111 \
            -exec scp {} \
            "${TARGET_USER}@${TARGET_HOST}:${TARGET_DIR}/bin/$dir/" \; \
            2>/dev/null || true

    fi

done

# --------------------------------------------------
# Deploy configuration
# --------------------------------------------------

echo "[5/5] Deploying configuration..."

if [ -f "$PROJECT_ROOT/configs/gpio-test-config.json" ]; then

    scp "$PROJECT_ROOT/configs/gpio-test-config.json" \
        "${TARGET_USER}@${TARGET_HOST}:${TARGET_DIR}/config/"

fi

echo

# --------------------------------------------------
# Set permissions
# --------------------------------------------------

ssh "${TARGET_USER}@${TARGET_HOST}" \
    "chmod -R +x $TARGET_DIR/bin"

echo
echo "=============================================="
echo " DEPLOYMENT COMPLETED"
echo "=============================================="
echo
echo "Target directory:"
echo "  $TARGET_DIR"
echo
echo "Connect using:"
echo
echo "  ssh ${TARGET_USER}@${TARGET_HOST}"
echo
echo "Then:"
echo
echo "  cd $TARGET_DIR"
echo
