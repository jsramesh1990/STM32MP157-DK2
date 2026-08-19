#!/bin/bash
#
# STM32MP157-DK2 GPIO Project
# Hardware Test Script
#

set -e

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

GPIO_CHIP="${GPIO_CHIP:-gpiochip0}"
LED_LINE="${LED_LINE:-17}"
BUTTON_LINE="${BUTTON_LINE:-18}"

PASS=0
FAIL=0

echo "=============================================="
echo " STM32MP157-DK2 GPIO Hardware Test"
echo "=============================================="
echo

echo "GPIO Chip   : $GPIO_CHIP"
echo "LED GPIO    : $LED_LINE"
echo "Button GPIO : $BUTTON_LINE"
echo

# --------------------------------------------------
# Helper functions
# --------------------------------------------------

pass_test()
{
    echo "[PASS] $1"
    PASS=$((PASS + 1))
}

fail_test()
{
    echo "[FAIL] $1"
    FAIL=$((FAIL + 1))
}

# --------------------------------------------------
# Check root
# --------------------------------------------------

echo "=== TEST 1: Permission ==="

if [ "$(id -u)" -eq 0 ]; then

    pass_test "Running as root"

else

    echo "[WARNING] Not running as root."

fi

echo

# --------------------------------------------------
# Check GPIO chip
# --------------------------------------------------

echo "=== TEST 2: GPIO CHIP ==="

if [ -e "/dev/$GPIO_CHIP" ]; then

    pass_test "/dev/$GPIO_CHIP exists"

else

    fail_test "/dev/$GPIO_CHIP not found"
fi

echo

# --------------------------------------------------
# Check gpiod tools
# --------------------------------------------------

echo "=== TEST 3: LIBGPIOD ==="

if command -v gpiodetect >/dev/null 2>&1; then

    echo
    gpiodetect
    echo

    pass_test "gpiodetect available"

else

    fail_test "gpiodetect not installed"
fi

echo

# --------------------------------------------------
# GPIO info
# --------------------------------------------------

echo "=== TEST 4: GPIO INFORMATION ==="

if command -v gpioinfo >/dev/null 2>&1; then

    gpioinfo "/dev/$GPIO_CHIP" >/dev/null 2>&1

    if [ $? -eq 0 ]; then
        pass_test "gpioinfo successfully accessed GPIO chip"
    else
        fail_test "gpioinfo failed"
    fi

else

    fail_test "gpioinfo unavailable"

fi

echo

# --------------------------------------------------
# LED output test
# --------------------------------------------------

echo "=== TEST 5: LED OUTPUT ==="

if command -v gpioset >/dev/null 2>&1; then

    echo "Turning LED GPIO ON..."

    gpioset "$GPIO_CHIP" "$LED_LINE=1" &
    GPIO_PID=$!

    sleep 2

    echo "Turning LED GPIO OFF..."

    kill "$GPIO_PID" 2>/dev/null || true

    wait "$GPIO_PID" 2>/dev/null || true

    pass_test "LED GPIO output test"

else

    fail_test "gpioset not available"

fi

echo

# --------------------------------------------------
# Button input test
# --------------------------------------------------

echo "=== TEST 6: BUTTON INPUT ==="

if command -v gpioget >/dev/null 2>&1; then

    VALUE=$(gpioget "$GPIO_CHIP" "$BUTTON_LINE" 2>/dev/null || echo "ERROR")

    if [ "$VALUE" != "ERROR" ]; then

        echo "Button GPIO value: $VALUE"

        pass_test "Button GPIO input test"

    else

        fail_test "Button GPIO read failed"

    fi

else

    fail_test "gpioget not available"

fi

echo

# --------------------------------------------------
# Application examples
# --------------------------------------------------

echo "=== TEST 7: APPLICATION EXAMPLES ==="

EXAMPLE_BIN="$PROJECT_ROOT/examples/led"

if [ -d "$EXAMPLE_BIN" ]; then

    echo "LED example directory found."

    if [ -f "$EXAMPLE_BIN/gpio_toggle" ]; then
        pass_test "gpio_toggle executable found"
    else
        echo "[INFO] gpio_toggle executable not built."
    fi

    if [ -f "$EXAMPLE_BIN/led_blink" ]; then
        pass_test "led_blink executable found"
    else
        echo "[INFO] led_blink executable not built."
    fi

else

    fail_test "LED example directory missing"
fi

echo

# --------------------------------------------------
# Interrupt test
# --------------------------------------------------

echo "=== TEST 8: GPIO INTERRUPT ==="

if [ -f "$PROJECT_ROOT/examples/interrupt/button_irq" ]; then

    pass_test "button_irq executable found"

elif [ -f "$PROJECT_ROOT/examples/interrupt/gpio_irq" ]; then

    pass_test "gpio_irq executable found"

else

    echo "[INFO] Interrupt executable not built."

fi

echo

# --------------------------------------------------
# PWM test
# --------------------------------------------------

echo "=== TEST 9: PWM ==="

if [ -f "$PROJECT_ROOT/examples/pwm/pwm_led" ]; then

    pass_test "pwm_led executable found"

elif [ -f "$PROJECT_ROOT/examples/pwm/pwm_fade" ]; then

    pass_test "pwm_fade executable found"

else

    echo "[INFO] PWM executable not built."

fi

echo

# --------------------------------------------------
# Kernel GPIO debug
# --------------------------------------------------

echo "=== TEST 10: KERNEL GPIO DEBUG ==="

if [ -f /sys/kernel/debug/gpio ]; then

    echo
    cat /sys/kernel/debug/gpio
    echo

    pass_test "Kernel GPIO debug information available"

else

    echo "[INFO] debugfs GPIO information unavailable."

fi

echo

# --------------------------------------------------
# Final report
# --------------------------------------------------

echo "=============================================="
echo " HARDWARE TEST SUMMARY"
echo "=============================================="
echo

echo "Tests Passed : $PASS"
echo "Tests Failed : $FAIL"

echo

if [ "$FAIL" -eq 0 ]; then

    echo "RESULT: ALL TESTS PASSED"

    exit 0

else

    echo "RESULT: SOME TESTS FAILED"

    exit 1

fi
