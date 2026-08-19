#!/bin/bash
#
# STM32MP157-DK2 GPIO Hardware Integration Test
#
# Purpose:
#   Validate GPIO operation on real STM32MP157-DK2 hardware
#   using the Linux GPIO character-device interface.
#
# Tests:
#   1. Detect GPIO chips
#   2. Display GPIO information
#   3. Configure GPIO as output
#   4. Set GPIO HIGH
#   5. Set GPIO LOW
#   6. Configure GPIO as input
#   7. Read GPIO value
#   8. Test GPIO toggle
#
# Usage:
#   ./test_gpio_hw.sh
#
# Optional:
#   GPIO_CHIP=gpiochip0 GPIO_LINE=13 ./test_gpio_hw.sh
#

set -u

# ------------------------------------------------------------
# Configuration
# ------------------------------------------------------------

GPIO_CHIP="${GPIO_CHIP:-gpiochip0}"
GPIO_LINE="${GPIO_LINE:-13}"

TEST_NAME="STM32MP157-DK2 GPIO Hardware Integration Test"

PASS=0
FAIL=0

# ------------------------------------------------------------
# Colors
# ------------------------------------------------------------

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# ------------------------------------------------------------
# Helper Functions
# ------------------------------------------------------------

print_header()
{
    echo
    echo "============================================================"
    echo " $TEST_NAME"
    echo "============================================================"
    echo
}

pass()
{
    echo -e "${GREEN}[PASS]${NC} $1"
    PASS=$((PASS + 1))
}

fail()
{
    echo -e "${RED}[FAIL]${NC} $1"
    FAIL=$((FAIL + 1))
}

info()
{
    echo -e "${BLUE}[INFO]${NC} $1"
}

warning()
{
    echo -e "${YELLOW}[WARN]${NC} $1"
}

command_exists()
{
    command -v "$1" >/dev/null 2>&1
}

# ------------------------------------------------------------
# Start
# ------------------------------------------------------------

print_header

info "GPIO chip : $GPIO_CHIP"
info "GPIO line : $GPIO_LINE"

# ------------------------------------------------------------
# Test 1: Check GPIO utilities
# ------------------------------------------------------------

echo
echo "------------------------------------------------------------"
echo "Test 1: Check GPIO utilities"
echo "------------------------------------------------------------"

if command_exists gpiodetect; then
    pass "gpiodetect is available"
else
    fail "gpiodetect is not installed"
fi

if command_exists gpioinfo; then
    pass "gpioinfo is available"
else
    fail "gpioinfo is not installed"
fi

if command_exists gpioset; then
    pass "gpioset is available"
else
    fail "gpioset is not installed"
fi

if command_exists gpioget; then
    pass "gpioget is available"
else
    fail "gpioget is not installed"
fi

# ------------------------------------------------------------
# Test 2: Detect GPIO chips
# ------------------------------------------------------------

echo
echo "------------------------------------------------------------"
echo "Test 2: Detect GPIO chips"
echo "------------------------------------------------------------"

if ! command_exists gpiodetect; then
    warning "Skipping GPIO chip detection"
else
    echo
    gpiodetect
    echo

    if gpiodetect | grep -q "$GPIO_CHIP"; then
        pass "$GPIO_CHIP detected"
    else
        fail "$GPIO_CHIP not detected"
    fi
fi

# ------------------------------------------------------------
# Test 3: GPIO information
# ------------------------------------------------------------

echo
echo "------------------------------------------------------------"
echo "Test 3: GPIO information"
echo "------------------------------------------------------------"

if command_exists gpioinfo; then

    if gpioinfo "$GPIO_CHIP" >/tmp/stm32_gpioinfo.txt 2>&1; then

        echo
        echo "GPIO information:"
        cat /tmp/stm32_gpioinfo.txt
        echo

        if grep -q "line $GPIO_LINE" /tmp/stm32_gpioinfo.txt; then
            pass "GPIO line $GPIO_LINE exists"
        else
            fail "GPIO line $GPIO_LINE not found"
        fi

    else
        fail "Unable to read GPIO information"
    fi

else
    warning "gpioinfo unavailable"
fi

# ------------------------------------------------------------
# Test 4: GPIO output HIGH
# ------------------------------------------------------------

echo
echo "------------------------------------------------------------"
echo "Test 4: GPIO output HIGH"
echo "------------------------------------------------------------"

if ! command_exists gpioset; then

    warning "gpioset unavailable - skipping"

else

    info "Setting $GPIO_CHIP line $GPIO_LINE HIGH"

    timeout 2 gpioset "$GPIO_CHIP" "$GPIO_LINE=1" >/dev/null 2>&1

    if [ $? -eq 0 ]; then
        pass "GPIO HIGH operation successful"
    else
        fail "GPIO HIGH operation failed"
    fi

fi

# ------------------------------------------------------------
# Test 5: GPIO output LOW
# ------------------------------------------------------------

echo
echo "------------------------------------------------------------"
echo "Test 5: GPIO output LOW"
echo "------------------------------------------------------------"

if ! command_exists gpioset; then

    warning "gpioset unavailable - skipping"

else

    info "Setting $GPIO_CHIP line $GPIO_LINE LOW"

    timeout 2 gpioset "$GPIO_CHIP" "$GPIO_LINE=0" >/dev/null 2>&1

    if [ $? -eq 0 ]; then
        pass "GPIO LOW operation successful"
    else
        fail "GPIO LOW operation failed"
    fi

fi

# ------------------------------------------------------------
# Test 6: GPIO input read
# ------------------------------------------------------------

echo
echo "------------------------------------------------------------"
echo "Test 6: GPIO input read"
echo "------------------------------------------------------------"

if ! command_exists gpioget; then

    warning "gpioget unavailable - skipping"

else

    info "Reading $GPIO_CHIP line $GPIO_LINE"

    GPIO_VALUE=$(gpioget "$GPIO_CHIP" "$GPIO_LINE" 2>/dev/null)

    if [ $? -eq 0 ]; then

        echo "GPIO value = $GPIO_VALUE"

        if [[ "$GPIO_VALUE" == "0" || "$GPIO_VALUE" == "1" ]]; then
            pass "GPIO input read successful"
        else
            fail "Invalid GPIO value: $GPIO_VALUE"
        fi

    else
        fail "GPIO input read failed"
    fi

fi

# ------------------------------------------------------------
# Test 7: GPIO toggle
# ------------------------------------------------------------

echo
echo "------------------------------------------------------------"
echo "Test 7: GPIO toggle"
echo "------------------------------------------------------------"

if ! command_exists gpioset; then

    warning "gpioset unavailable - skipping"

else

    info "Starting GPIO toggle test"

    TOGGLE_FAILED=0

    for i in 1 2 3 4 5
    do
        info "Toggle $i: HIGH"

        timeout 1 gpioset "$GPIO_CHIP" "$GPIO_LINE=1" >/dev/null 2>&1

        if [ $? -ne 0 ]; then
            TOGGLE_FAILED=1
            break
        fi

        sleep 0.2

        info "Toggle $i: LOW"

        timeout 1 gpioset "$GPIO_CHIP" "$GPIO_LINE=0" >/dev/null 2>&1

        if [ $? -ne 0 ]; then
            TOGGLE_FAILED=1
            break
        fi

        sleep 0.2
    done

    if [ "$TOGGLE_FAILED" -eq 0 ]; then
        pass "GPIO toggle test successful"
    else
        fail "GPIO toggle test failed"
    fi

fi

# ------------------------------------------------------------
# Test 8: Check GPIO device
# ------------------------------------------------------------

echo
echo "------------------------------------------------------------"
echo "Test 8: GPIO device node"
echo "------------------------------------------------------------"

GPIO_DEVICE="/dev/$GPIO_CHIP"

if [ -e "$GPIO_DEVICE" ]; then
    pass "$GPIO_DEVICE exists"
else
    fail "$GPIO_DEVICE does not exist"
fi

# ------------------------------------------------------------
# Cleanup
# ------------------------------------------------------------

rm -f /tmp/stm32_gpioinfo.txt

# ------------------------------------------------------------
# Test Summary
# ------------------------------------------------------------

echo
echo "============================================================"
echo " TEST SUMMARY"
echo "============================================================"

echo
echo -e "${GREEN}Passed : $PASS${NC}"
echo -e "${RED}Failed : $FAIL${NC}"
echo

TOTAL=$((PASS + FAIL))

echo "Total tests: $TOTAL"

echo
echo "============================================================"

if [ "$FAIL" -eq 0 ]; then

    echo -e "${GREEN}GPIO HARDWARE TEST: PASSED${NC}"

    echo
    echo "STM32MP157-DK2 GPIO hardware validation completed."
    echo

    exit 0

else

    echo -e "${RED}GPIO HARDWARE TEST: FAILED${NC}"

    echo
    echo "Please check:"
    echo "  1. GPIO pin configuration"
    echo "  2. Device Tree configuration"
    echo "  3. GPIO controller status"
    echo "  4. GPIO pin ownership"
    echo "  5. Hardware connections"
    echo "  6. User permissions"
    echo "  7. libgpiod installation"
    echo

    exit 1

fi
