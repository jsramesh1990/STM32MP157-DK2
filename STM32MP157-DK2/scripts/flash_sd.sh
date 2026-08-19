#!/bin/bash
#
# STM32MP157-DK2 GPIO Project
# SD Card Flash Script
#

set -e

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

IMAGE_DIR="${IMAGE_DIR:-$PROJECT_ROOT/images}"
SD_DEVICE="${1:-}"

echo "=============================================="
echo " STM32MP157-DK2 SD Card Flash"
echo "=============================================="
echo

if [ "$(id -u)" -ne 0 ]; then
    echo "[ERROR] This script must be run as root."
    echo
    echo "Use:"
    echo "  sudo ./scripts/flash_sd.sh /dev/sdX"
    exit 1
fi

if [ -z "$SD_DEVICE" ]; then
    echo "[ERROR] No SD device specified."
    echo
    echo "Usage:"
    echo
    echo "  sudo ./scripts/flash_sd.sh /dev/sdX"
    echo
    echo "Available devices:"
    lsblk
    exit 1
fi

# --------------------------------------------------
# Safety checks
# --------------------------------------------------

if [ ! -b "$SD_DEVICE" ]; then
    echo "[ERROR] $SD_DEVICE is not a block device."
    exit 1
fi

if [[ "$SD_DEVICE" == "/"* ]]; then
    :
else
    echo "[ERROR] Invalid device."
    exit 1
fi

echo "[WARNING]"
echo "You are about to erase:"
echo
lsblk "$SD_DEVICE"
echo
echo "ALL DATA ON THIS DEVICE MAY BE LOST."
echo

read -rp "Type FLASH to continue: " CONFIRM

if [ "$CONFIRM" != "FLASH" ]; then
    echo "Flash cancelled."
    exit 0
fi

# --------------------------------------------------
# Unmount partitions
# --------------------------------------------------

echo
echo "[1/5] Unmounting SD card partitions..."

for partition in $(lsblk -lnpo NAME "$SD_DEVICE" | tail -n +2); do

    umount "$partition" 2>/dev/null || true

done

# --------------------------------------------------
# Check image
# --------------------------------------------------

echo "[2/5] Searching for image..."

IMAGE=""

for file in \
    "$IMAGE_DIR/stm32mp157-gpio-test.img" \
    "$IMAGE_DIR/stm32mp157-gpio-test.wic" \
    "$IMAGE_DIR/stm32mp157-dk2-image.wic" \
    "$IMAGE_DIR/stm32mp157-dk2-image.img"
do

    if [ -f "$file" ]; then
        IMAGE="$file"
        break
    fi

done

if [ -z "$IMAGE" ]; then
    echo "[ERROR] No SD image found."
    echo
    echo "Expected image directory:"
    echo "  $IMAGE_DIR"
    echo
    echo "Place a .wic or .img file there."
    exit 1
fi

echo "[OK] Image:"
echo "     $IMAGE"

# --------------------------------------------------
# Flash image
# --------------------------------------------------

echo
echo "[3/5] Flashing image..."

dd if="$IMAGE" \
   of="$SD_DEVICE" \
   bs=4M \
   status=progress \
   conv=fsync

# --------------------------------------------------
# Sync
# --------------------------------------------------

echo
echo "[4/5] Synchronizing data..."

sync

# --------------------------------------------------
# Verify
# --------------------------------------------------

echo
echo "[5/5] Flash operation completed."

echo
echo "=============================================="
echo " SD CARD FLASH COMPLETED"
echo "=============================================="
echo
echo "Next steps:"
echo
echo "1. Remove SD card safely."
echo "2. Insert SD card into STM32MP157-DK2."
echo "3. Set board boot configuration for SD boot."
echo "4. Power ON the board."
echo "5. Open UART console."
echo
echo "Example UART:"
echo "  115200 baud"
echo "  8 data bits"
echo "  1 stop bit"
echo "  No parity"
echo
