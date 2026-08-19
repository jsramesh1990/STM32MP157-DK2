#!/bin/bash
#
# STM32MP157-DK2 GPIO Project
# GPIO Information Script
#

echo "=============================================="
echo " STM32MP157-DK2 GPIO Information"
echo "=============================================="
echo

# --------------------------------------------------
# System information
# --------------------------------------------------

echo "=== SYSTEM ==="
echo

echo "Kernel:"
uname -a

echo
echo "Machine:"
uname -m

echo
echo "Hostname:"
hostname

echo

# --------------------------------------------------
# GPIO devices
# --------------------------------------------------

echo "=== GPIO CHIPS ==="
echo

if ls /dev/gpiochip* >/dev/null 2>&1; then

    ls -l /dev/gpiochip*

else

    echo "[WARNING] No /dev/gpiochip devices found."

fi

echo

# --------------------------------------------------
# gpiodetect
# --------------------------------------------------

echo "=== GPIODetect ==="
echo

if command -v gpiodetect >/dev/null 2>&1; then

    gpiodetect

else

    echo "[WARNING] gpiodetect not installed."

fi

echo

# --------------------------------------------------
# gpioinfo
# --------------------------------------------------

echo "=== GPIOINFO ==="
echo

if command -v gpioinfo >/dev/null 2>&1; then

    for chip in /dev/gpiochip*; do

        echo
        echo "--- $chip ---"

        gpioinfo "$chip" || true

    done

else

    echo "[WARNING] gpioinfo not installed."

fi

echo

# --------------------------------------------------
# GPIO debug filesystem
# --------------------------------------------------

echo "=== KERNEL GPIO DEBUG ==="
echo

if [ -f /sys/kernel/debug/gpio ]; then

    cat /sys/kernel/debug/gpio

else

    echo "[INFO] /sys/kernel/debug/gpio unavailable."
    echo "[INFO] Try:"
    echo "       mount -t debugfs none /sys/kernel/debug"

fi

echo

# --------------------------------------------------
# GPIO sysfs
# --------------------------------------------------

echo "=== LEGACY SYSFS GPIO ==="
echo

if [ -d /sys/class/gpio ]; then

    ls -la /sys/class/gpio/

else

    echo "[INFO] /sys/class/gpio not available."
    echo "[INFO] Sysfs GPIO may be disabled or deprecated."

fi

echo

# --------------------------------------------------
# Device Tree
# --------------------------------------------------

echo "=== DEVICE TREE GPIO ==="
echo

if [ -d /proc/device-tree ]; then

    echo "Device Tree available:"
    echo "  /proc/device-tree"

    echo
    echo "GPIO-related nodes:"

    find /proc/device-tree \
        -iname "*gpio*" \
        -maxdepth 5 \
        2>/dev/null | head -50

else

    echo "[WARNING] Device Tree not mounted."
fi

echo

# --------------------------------------------------
# GPIO kernel configuration
# --------------------------------------------------

echo "=== KERNEL GPIO CONFIG ==="
echo

if [ -f /proc/config.gz ]; then

    zcat /proc/config.gz | grep -E "CONFIG_GPIOLIB|CONFIG_GPIO"

elif [ -f "/boot/config-$(uname -r)" ]; then

    grep -E "CONFIG_GPIOLIB|CONFIG_GPIO" \
        "/boot/config-$(uname -r)" || true

else

    echo "[INFO] Kernel configuration not available."

fi

echo

# --------------------------------------------------
# Summary
# --------------------------------------------------

echo "=============================================="
echo " GPIO INFORMATION COMPLETE"
echo "=============================================="
