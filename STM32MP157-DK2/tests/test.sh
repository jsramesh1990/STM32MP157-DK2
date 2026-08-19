#!/bin/bash
#
# STM32MP157-DK2 GPIO Test Framework
#
# Top-level test runner for:
#   - Unit tests
#   - Simulation tests
#   - Hardware integration tests
#
# Usage:
#   ./test.sh
#   ./test.sh unit
#   ./test.sh simulation
#   ./test.sh integration
#   ./test.sh all
#   ./test.sh help
#

set -u

# ============================================================
# Project Paths
# ============================================================

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"

UNIT_DIR="${SCRIPT_DIR}/unit"
SIM_DIR="${SCRIPT_DIR}/simulation"
INTEGRATION_DIR="${SCRIPT_DIR}/integration"

UNIT_TEST="${UNIT_DIR}/test_gpio"
SIM_TEST="${SIM_DIR}/test_simulator.sh"
HW_TEST="${INTEGRATION_DIR}/test_gpio_hw.sh"

# ============================================================
# Colors
# ============================================================

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
NC='\033[0m'

# ============================================================
# Test Counters
# ============================================================

TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0
SKIPPED_TESTS=0

# ============================================================
# Helper Functions
# ============================================================

print_header()
{
    echo
    echo "============================================================"
    echo "        STM32MP157-DK2 GPIO TEST FRAMEWORK"
    echo "============================================================"
    echo
}

print_section()
{
    echo
    echo -e "${BLUE}------------------------------------------------------------${NC}"
    echo -e "${BLUE}$1${NC}"
    echo -e "${BLUE}------------------------------------------------------------${NC}"
    echo
}

pass()
{
    echo -e "${GREEN}[PASS]${NC} $1"
    PASSED_TESTS=$((PASSED_TESTS + 1))
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
}

fail()
{
    echo -e "${RED}[FAIL]${NC} $1"
    FAILED_TESTS=$((FAILED_TESTS + 1))
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
}

skip()
{
    echo -e "${YELLOW}[SKIP]${NC} $1"
    SKIPPED_TESTS=$((SKIPPED_TESTS + 1))
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
}

info()
{
    echo -e "${CYAN}[INFO]${NC} $1"
}

error()
{
    echo -e "${RED}[ERROR]${NC} $1"
}

# ============================================================
# Check Project Structure
# ============================================================

check_project_structure()
{
    print_section "Checking Project Structure"

    local required_dirs=(
        "${PROJECT_ROOT}/application"
        "${PROJECT_ROOT}/application/include"
        "${PROJECT_ROOT}/application/src"
        "${PROJECT_ROOT}/application/examples"
        "${PROJECT_ROOT}/configs"
        "${PROJECT_ROOT}/device-tree"
        "${PROJECT_ROOT}/docs"
        "${PROJECT_ROOT}/examples"
        "${PROJECT_ROOT}/kernel"
        "${PROJECT_ROOT}/kernel/driver"
        "${PROJECT_ROOT}/kernel/patches"
        "${PROJECT_ROOT}/scripts"
        "${PROJECT_ROOT}/simulator"
        "${PROJECT_ROOT}/tests"
    )

    local dir

    for dir in "${required_dirs[@]}"; do
        if [ -d "$dir" ]; then
            echo -e "${GREEN}[OK]${NC} $(basename "$dir")"
        else
            echo -e "${YELLOW}[WARN]${NC} Missing directory: $dir"
        fi
    done

    echo
}

# ============================================================
# Check Dependencies
# ============================================================

check_dependencies()
{
    print_section "Checking Host Dependencies"

    local commands=(
        gcc
        make
        bash
        python3
    )

    local cmd

    for cmd in "${commands[@]}"; do
        if command -v "$cmd" >/dev/null 2>&1; then
            echo -e "${GREEN}[OK]${NC} $cmd"
        else
            echo -e "${YELLOW}[WARN]${NC} $cmd not found"
        fi
    done

    echo

    # libgpiod is optional for simulation but required for
    # real GPIO access.
    if command -v gpiodetect >/dev/null 2>&1; then
        echo -e "${GREEN}[OK]${NC} libgpiod tools available"
    else
        echo -e "${YELLOW}[WARN]${NC} libgpiod tools not found"
        echo "      Install with:"
        echo "      sudo apt install gpiod libgpiod-dev"
    fi

    echo
}

# ============================================================
# Build Unit Test
# ============================================================

build_unit_test()
{
    print_section "Building Unit Test"

    if [ ! -f "${UNIT_DIR}/test_gpio.c" ]; then
        error "Unit test source not found:"
        echo "  ${UNIT_DIR}/test_gpio.c"
        return 1
    fi

    info "Compiling test_gpio.c"

    gcc \
        -Wall \
        -Wextra \
        -g \
        -I"${PROJECT_ROOT}/application/include" \
        -I"${UNIT_DIR}" \
        "${UNIT_DIR}/test_gpio.c" \
        "${PROJECT_ROOT}/application/src/gpio-common.c" \
        "${PROJECT_ROOT}/application/src/gpio-sysfs.c" \
        -o "${UNIT_TEST}" \
        2>&1

    if [ $? -eq 0 ]; then
        pass "Unit test build"
        return 0
    else
        fail "Unit test build"
        return 1
    fi
}

# ============================================================
# Run Unit Tests
# ============================================================

run_unit_tests()
{
    print_section "Running Unit Tests"

    if [ ! -x "${UNIT_TEST}" ]; then
        if ! build_unit_test; then
            return 1
        fi
    fi

    info "Executing unit test:"
    echo "  ${UNIT_TEST}"
    echo

    "${UNIT_TEST}"

    local ret=$?

    if [ $ret -eq 0 ]; then
        pass "Unit test execution"
        return 0
    else
        fail "Unit test execution"
        return 1
    fi
}

# ============================================================
# Run Simulation Tests
# ============================================================

run_simulation_tests()
{
    print_section "Running Simulation Tests"

    if [ ! -f "${SIM_TEST}" ]; then
        error "Simulation test not found:"
        echo "  ${SIM_TEST}"
        fail "Simulation test availability"
        return 1
    fi

    if [ ! -x "${SIM_TEST}" ]; then
        info "Making simulation test executable"
        chmod +x "${SIM_TEST}"
    fi

    info "Executing simulation test:"
    echo "  ${SIM_TEST}"
    echo

    "${SIM_TEST}"

    local ret=$?

    if [ $ret -eq 0 ]; then
        pass "Simulation test execution"
        return 0
    else
        fail "Simulation test execution"
        return 1
    fi
}

# ============================================================
# Detect STM32MP157 Hardware
# ============================================================

detect_hardware()
{
    info "Checking for STM32MP157 GPIO hardware..."

    # Check GPIO character devices
    if ls /dev/gpiochip* >/dev/null 2>&1; then
        echo
        echo "GPIO chips detected:"
        ls -l /dev/gpiochip*
        echo

        return 0
    fi

    # Check legacy sysfs GPIO
    if [ -d "/sys/class/gpio" ]; then
        info "Legacy GPIO sysfs interface detected"
        return 0
    fi

    return 1
}

# ============================================================
# Run Hardware Integration Tests
# ============================================================

run_integration_tests()
{
    print_section "Running Hardware Integration Tests"

    if [ ! -f "${HW_TEST}" ]; then
        error "Hardware test not found:"
        echo "  ${HW_TEST}"
        fail "Hardware test availability"
        return 1
    fi

    if ! detect_hardware; then
        skip "STM32MP157 GPIO hardware not detected"
        echo
        echo "Hardware test requires the STM32MP157-DK2 board."
        echo "Run this test on the target board."
        return 0
    fi

    if [ ! -x "${HW_TEST}" ]; then
        info "Making hardware test executable"
        chmod +x "${HW_TEST}"
    fi

    info "Executing hardware integration test:"
    echo "  ${HW_TEST}"
    echo

    "${HW_TEST}"

    local ret=$?

    if [ $ret -eq 0 ]; then
        pass "Hardware integration test"
        return 0
    else
        fail "Hardware integration test"
        return 1
    fi
}

# ============================================================
# Run GPIO Information
# ============================================================

show_gpio_information()
{
    print_section "GPIO Information"

    if command -v gpiodetect >/dev/null 2>&1; then

        echo "GPIO CHIPS:"
        echo "------------"
        gpiodetect
        echo

        echo "GPIO INFORMATION:"
        echo "-----------------"

        local chip

        for chip in /dev/gpiochip*; do
            if [ -e "$chip" ]; then
                gpioinfo "$chip" 2>/dev/null || true
                echo
            fi
        done

    else
        info "gpiodetect not available"

        if [ -d "/sys/class/gpio" ]; then
            echo
            echo "Legacy GPIO interface:"
            ls -la /sys/class/gpio/
        fi
    fi
}

# ============================================================
# Run Application Build Test
# ============================================================

run_build_test()
{
    print_section "Application Build Test"

    local makefile="${PROJECT_ROOT}/Makefile"

    if [ -f "$makefile" ]; then

        info "Building project using top-level Makefile"

        (
            cd "${PROJECT_ROOT}" || exit 1
            make clean
            make
        )

        local ret=$?

        if [ $ret -eq 0 ]; then
            pass "Application build"
            return 0
        else
            fail "Application build"
            return 1
        fi

    else
        info "Top-level Makefile not found"

        # Build individual application sources if available.
        if [ -f "${PROJECT_ROOT}/application/src/gpio-common.c" ]; then
            info "Application source exists"
            pass "Application source verification"
            return 0
        fi

        skip "Application build"
        return 0
    fi
}

# ============================================================
# Run All Tests
# ============================================================

run_all_tests()
{
    print_header

    check_project_structure
    check_dependencies

    # --------------------------------------------------------
    # Build
    # --------------------------------------------------------

    run_build_test

    # --------------------------------------------------------
    # Unit
    # --------------------------------------------------------

    run_unit_tests

    # --------------------------------------------------------
    # Simulation
    # --------------------------------------------------------

    run_simulation_tests

    # --------------------------------------------------------
    # Hardware
    # --------------------------------------------------------

    run_integration_tests

    # --------------------------------------------------------
    # GPIO Information
    # --------------------------------------------------------

    show_gpio_information

    # --------------------------------------------------------
    # Summary
    # --------------------------------------------------------

    print_summary
}

# ============================================================
# Test Summary
# ============================================================

print_summary()
{
    echo
    echo "============================================================"
    echo "                    TEST SUMMARY"
    echo "============================================================"
    echo

    echo "Total Tests   : ${TOTAL_TESTS}"
    echo -e "Passed        : ${GREEN}${PASSED_TESTS}${NC}"
    echo -e "Failed        : ${RED}${FAILED_TESTS}${NC}"
    echo -e "Skipped       : ${YELLOW}${SKIPPED_TESTS}${NC}"

    echo
    echo "============================================================"

    if [ "${FAILED_TESTS}" -eq 0 ]; then
        echo -e "${GREEN}RESULT: TEST SUITE PASSED${NC}"
        echo "============================================================"
        return 0
    else
        echo -e "${RED}RESULT: TEST SUITE FAILED${NC}"
        echo "============================================================"
        return 1
    fi
}

# ============================================================
# Help
# ============================================================

show_help()
{
    cat << EOF

STM32MP157-DK2 GPIO Test Framework
===================================

Usage:
    ./test.sh [command]

Commands:

    all
        Run complete test suite.
        Includes build, unit, simulation and hardware tests.

    unit
        Run unit tests only.

    simulation
        Run GPIO simulator tests only.

    integration
        Run STM32MP157-DK2 hardware integration tests.

    build
        Build application and test components.

    gpio-info
        Display available GPIO chips and GPIO lines.

    dependencies
        Check required host dependencies.

    structure
        Check project directory structure.

    help
        Display this help message.

Examples:

    ./test.sh
    ./test.sh all
    ./test.sh unit
    ./test.sh simulation
    ./test.sh integration
    ./test.sh build
    ./test.sh gpio-info

For hardware testing:

    1. Boot STM32MP157-DK2
    2. Login through serial console or SSH
    3. Verify /dev/gpiochip* exists
    4. Run:

       sudo ./test.sh integration

EOF
}

# ============================================================
# Main
# ============================================================

main()
{
    local command="${1:-all}"

    case "${command}" in

        all)
            run_all_tests
            ;;

        unit)
            print_header
            check_dependencies
            run_unit_tests
            print_summary
            ;;

        simulation)
            print_header
            check_dependencies
            run_simulation_tests
            print_summary
            ;;

        integration|hardware)
            print_header
            check_dependencies
            run_integration_tests
            print_summary
            ;;

        build)
            print_header
            check_dependencies
            run_build_test
            print_summary
            ;;

        gpio-info)
            print_header
            show_gpio_information
            ;;

        dependencies)
            print_header
            check_dependencies
            ;;

        structure)
            print_header
            check_project_structure
            ;;

        help|-h|--help)
            show_help
            ;;

        *)
            error "Unknown command: ${command}"
            echo
            show_help
            exit 1
            ;;
    esac
}

# ============================================================
# Execute
# ============================================================

main "$@"
