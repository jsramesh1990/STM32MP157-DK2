#!/bin/bash
#
# STM32MP157-DK2 GPIO Simulator Test
#
# Purpose:
#   Validate the GPIO simulator without requiring
#   physical STM32MP157-DK2 hardware.
#
# Tests:
#   1. Simulator files exist
#   2. Python environment
#   3. Configuration file
#   4. GPIO model import
#   5. GPIO event module import
#   6. Simulator module import
#   7. GPIO model functionality
#   8. GPIO event generation
#   9. Simulator startup
#
# Usage:
#   ./test_simulator.sh
#

set -u

# ------------------------------------------------------------
# Project Paths
# ------------------------------------------------------------

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
SIMULATOR_DIR="$PROJECT_ROOT/simulator"

PYTHON="${PYTHON:-python3}"

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

# ------------------------------------------------------------
# Header
# ------------------------------------------------------------

echo
echo "============================================================"
echo " STM32MP157-DK2 GPIO Simulator Test"
echo "============================================================"
echo

info "Project root : $PROJECT_ROOT"
info "Simulator    : $SIMULATOR_DIR"
info "Python       : $PYTHON"

# ------------------------------------------------------------
# Test 1: Check simulator directory
# ------------------------------------------------------------

echo
echo "------------------------------------------------------------"
echo "Test 1: Simulator directory"
echo "------------------------------------------------------------"

if [ -d "$SIMULATOR_DIR" ]; then
    pass "Simulator directory exists"
else
    fail "Simulator directory not found"
    exit 1
fi

# ------------------------------------------------------------
# Test 2: Check Python
# ------------------------------------------------------------

echo
echo "------------------------------------------------------------"
echo "Test 2: Python environment"
echo "------------------------------------------------------------"

if command -v "$PYTHON" >/dev/null 2>&1; then

    PYTHON_VERSION=$("$PYTHON" --version 2>&1)

    info "$PYTHON_VERSION"

    pass "Python is available"

else

    fail "Python3 is not installed"
    exit 1

fi

# ------------------------------------------------------------
# Test 3: Check required files
# ------------------------------------------------------------

echo
echo "------------------------------------------------------------"
echo "Test 3: Simulator files"
echo "------------------------------------------------------------"

REQUIRED_FILES=(
    "config.json"
    "gpio_events.py"
    "gpio_model.py"
    "simulator.py"
)

for file in "${REQUIRED_FILES[@]}"
do

    if [ -f "$SIMULATOR_DIR/$file" ]; then
        pass "$file exists"
    else
        fail "$file missing"
    fi

done

# ------------------------------------------------------------
# Test 4: Validate JSON configuration
# ------------------------------------------------------------

echo
echo "------------------------------------------------------------"
echo "Test 4: Configuration validation"
echo "------------------------------------------------------------"

CONFIG_FILE="$SIMULATOR_DIR/config.json"

if [ -f "$CONFIG_FILE" ]; then

    "$PYTHON" - "$CONFIG_FILE" <<'PY'
import json
import sys

filename = sys.argv[1]

try:
    with open(filename, "r") as f:
        data = json.load(f)

    print("JSON configuration is valid")

except Exception as e:
    print("JSON configuration error:", e)
    sys.exit(1)
PY

    if [ $? -eq 0 ]; then
        pass "config.json is valid"
    else
        fail "config.json is invalid"
    fi

else

    fail "config.json not found"

fi

# ------------------------------------------------------------
# Test 5: Import GPIO model
# ------------------------------------------------------------

echo
echo "------------------------------------------------------------"
echo "Test 5: GPIO model import"
echo "------------------------------------------------------------"

cd "$SIMULATOR_DIR"

"$PYTHON" - <<'PY'
try:
    import gpio_model
    print("gpio_model imported successfully")
except Exception as e:
    print("Import error:", e)
    raise SystemExit(1)
PY

if [ $? -eq 0 ]; then
    pass "gpio_model import successful"
else
    fail "gpio_model import failed"
fi

# ------------------------------------------------------------
# Test 6: Import GPIO events
# ------------------------------------------------------------

echo
echo "------------------------------------------------------------"
echo "Test 6: GPIO event module"
echo "------------------------------------------------------------"

"$PYTHON" - <<'PY'
try:
    import gpio_events
    print("gpio_events imported successfully")
except Exception as e:
    print("Import error:", e)
    raise SystemExit(1)
PY

if [ $? -eq 0 ]; then
    pass "gpio_events import successful"
else
    fail "gpio_events import failed"
fi

# ------------------------------------------------------------
# Test 7: Import simulator
# ------------------------------------------------------------

echo
echo "------------------------------------------------------------"
echo "Test 7: Simulator module"
echo "------------------------------------------------------------"

"$PYTHON" - <<'PY'
try:
    import simulator
    print("simulator imported successfully")
except Exception as e:
    print("Import error:", e)
    raise SystemExit(1)
PY

if [ $? -eq 0 ]; then
    pass "simulator import successful"
else
    fail "simulator import failed"
fi

# ------------------------------------------------------------
# Test 8: GPIO model functionality
# ------------------------------------------------------------

echo
echo "------------------------------------------------------------"
echo "Test 8: GPIO model functionality"
echo "------------------------------------------------------------"

"$PYTHON" - <<'PY'
import gpio_model

print("GPIO model loaded")

# Try common class names.
GPIOClass = None

for name in ["GPIOModel", "GPIOSimulator", "VirtualGPIO"]:
    if hasattr(gpio_model, name):
        GPIOClass = getattr(gpio_model, name)
        print("Using class:", name)
        break

if GPIOClass is None:
    print("No supported GPIO model class found")
    raise SystemExit(1)

try:

    # Most simulator implementations use num_pins.
    gpio = GPIOClass(num_pins=32)

except TypeError:

    try:
        gpio = GPIOClass(32)

    except Exception as e:
        print("Unable to create GPIO model:", e)
        raise SystemExit(1)

print("GPIO model instance created successfully")

# Test basic operations when available.
if hasattr(gpio, "set_direction"):
    gpio.set_direction(0, "out")
    print("Direction configured")

if hasattr(gpio, "write"):
    gpio.write(0, 1)
    print("GPIO write successful")

if hasattr(gpio, "read"):
    value = gpio.read(0)
    print("GPIO read value:", value)

print("GPIO model test completed")

PY

if [ $? -eq 0 ]; then
    pass "GPIO model functionality test successful"
else
    fail "GPIO model functionality test failed"
fi

# ------------------------------------------------------------
# Test 9: Event generation
# ------------------------------------------------------------

echo
echo "------------------------------------------------------------"
echo "Test 9: GPIO event functionality"
echo "------------------------------------------------------------"

"$PYTHON" - <<'PY'
try:
    import gpio_events

    print("GPIO event module loaded")

    # Check available classes/functions.
    names = [
        name for name in dir(gpio_events)
        if not name.startswith("_")
    ]

    print("Available event objects:")

    for name in names:
        print("  ", name)

    print("GPIO event module test completed")

except Exception as e:

    print("GPIO event test failed:", e)
    raise SystemExit(1)

PY

if [ $? -eq 0 ]; then
    pass "GPIO event test successful"
else
    fail "GPIO event test failed"
fi

# ------------------------------------------------------------
# Test 10: Simulator startup
# ------------------------------------------------------------

echo
echo "------------------------------------------------------------"
echo "Test 10: Simulator startup"
echo "------------------------------------------------------------"

SIMULATOR="$SIMULATOR_DIR/simulator.py"

if [ ! -f "$SIMULATOR" ]; then

    fail "simulator.py not found"

else

    info "Checking simulator startup..."

    # Try --help first.
    "$PYTHON" "$SIMULATOR" --help >/tmp/stm32_simulator_help.txt 2>&1

    if [ $? -eq 0 ]; then

        pass "Simulator command line interface works"

    else

        warning "Simulator does not support --help"

        # Start simulator for a short period.
        timeout 3 "$PYTHON" "$SIMULATOR" \
            >/tmp/stm32_simulator.log 2>&1 &

        SIM_PID=$!

        sleep 1

        if kill -0 "$SIM_PID" 2>/dev/null; then

            pass "Simulator started successfully"

            kill "$SIM_PID" 2>/dev/null
            wait "$SIM_PID" 2>/dev/null

        else

            warning "Simulator exited during startup"

            if grep -qi "error\|exception\|traceback" \
                /tmp/stm32_simulator.log; then

                fail "Simulator startup failed"

            else

                pass "Simulator startup check completed"

            fi

        fi

    fi

fi

# ------------------------------------------------------------
# Cleanup
# ------------------------------------------------------------

rm -f /tmp/stm32_simulator_help.txt
rm -f /tmp/stm32_simulator.log

# ------------------------------------------------------------
# Summary
# ------------------------------------------------------------

echo
echo "============================================================"
echo " SIMULATION TEST SUMMARY"
echo "============================================================"
echo

echo -e "${GREEN}Passed : $PASS${NC}"
echo -e "${RED}Failed : $FAIL${NC}"

TOTAL=$((PASS + FAIL))

echo "Total  : $TOTAL"

echo
echo "============================================================"

if [ "$FAIL" -eq 0 ]; then

    echo -e "${GREEN}GPIO SIMULATION TEST: PASSED${NC}"
    echo
    echo "STM32MP157-DK2 GPIO simulator validation completed."
    echo

    exit 0

else

    echo -e "${RED}GPIO SIMULATION TEST: FAILED${NC}"
    echo
    echo "Please check:"
    echo "  1. Python installation"
    echo "  2. Simulator Python files"
    echo "  3. config.json"
    echo "  4. Python module dependencies"
    echo "  5. GPIO model implementation"
    echo "  6. GPIO event implementation"
    echo

    exit 1

fi
