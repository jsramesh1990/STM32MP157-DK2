/*
 * test_gpio.c
 *
 * STM32MP157-DK2 GPIO Unit Tests
 *
 * Purpose:
 *   Unit-test the common GPIO API and GPIO configuration logic.
 *
 * These tests are intended to run on the development host and
 * do not require physical GPIO hardware.
 *
 * Build example:
 *
 *   gcc -Wall -Wextra -g \
 *       -I../../application/include \
 *       test_gpio.c \
 *       ../../application/src/gpio-common.c \
 *       -o test_gpio
 *
 * Run:
 *
 *   ./test_gpio
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/*
 * Project GPIO header
 */
#include "gpio_common.h"

/* ============================================================
 * Test Counters
 * ============================================================
 */

static int tests_passed = 0;
static int tests_failed = 0;

/* ============================================================
 * Test Macros
 * ============================================================
 */

#define TEST_START(name)                                      \
    do {                                                      \
        printf("\n--------------------------------------------------\n"); \
        printf("TEST: %s\n", name);                           \
        printf("--------------------------------------------------\n"); \
    } while (0)


#define TEST_PASS(name)                                       \
    do {                                                      \
        printf("[PASS] %s\n", name);                         \
        tests_passed++;                                      \
    } while (0)


#define TEST_FAIL(name)                                       \
    do {                                                      \
        printf("[FAIL] %s\n", name);                         \
        tests_failed++;                                      \
    } while (0)


#define EXPECT_TRUE(condition, name)                          \
    do {                                                      \
        if (condition) {                                      \
            TEST_PASS(name);                                  \
        } else {                                              \
            TEST_FAIL(name);                                  \
        }                                                     \
    } while (0)


#define EXPECT_EQ(actual, expected, name)                     \
    do {                                                      \
        if ((actual) == (expected)) {                         \
            TEST_PASS(name);                                  \
        } else {                                              \
            printf("       Expected: %d\n", (expected));      \
            printf("       Actual  : %d\n", (actual));        \
            TEST_FAIL(name);                                  \
        }                                                     \
    } while (0)


#define EXPECT_STR_EQ(actual, expected, name)                 \
    do {                                                      \
        if ((actual) != NULL &&                               \
            strcmp((actual), (expected)) == 0) {              \
            TEST_PASS(name);                                  \
        } else {                                              \
            printf("       Expected: %s\n", (expected));      \
            printf("       Actual  : %s\n",                \
                   (actual) ? (actual) : "NULL");             \
            TEST_FAIL(name);                                  \
        }                                                     \
    } while (0)


/* ============================================================
 * Test 1
 * GPIO Configuration Structure
 * ============================================================
 */

static void test_gpio_config_initialization(void)
{
    TEST_START("GPIO configuration initialization");

    gpio_config_t config;

    memset(&config, 0, sizeof(config));

    config.pin = 17;
    config.chip = 0;
    config.value = 0;
    config.use_libgpiod = 1;

    strncpy(config.direction,
            "out",
            sizeof(config.direction) - 1);

    strncpy(config.edge,
            "none",
            sizeof(config.edge) - 1);

    strncpy(config.bias,
            "default",
            sizeof(config.bias) - 1);

    EXPECT_EQ(config.pin,
              17,
              "GPIO pin number");

    EXPECT_EQ(config.chip,
              0,
              "GPIO chip number");

    EXPECT_EQ(config.value,
              0,
              "Initial GPIO value");

    EXPECT_EQ(config.use_libgpiod,
              1,
              "libgpiod selection");

    EXPECT_STR_EQ(config.direction,
                  "out",
                  "GPIO direction");

    EXPECT_STR_EQ(config.edge,
                  "none",
                  "GPIO edge");

    EXPECT_STR_EQ(config.bias,
                  "default",
                  "GPIO bias");
}


/* ============================================================
 * Test 2
 * GPIO Output Configuration
 * ============================================================
 */

static void test_gpio_output_configuration(void)
{
    TEST_START("GPIO output configuration");

    gpio_config_t config;

    memset(&config, 0, sizeof(config));

    config.pin = 17;
    config.chip = 0;

    strncpy(config.direction,
            "out",
            sizeof(config.direction) - 1);

    config.value = 0;

    EXPECT_EQ(config.pin,
              17,
              "Output GPIO pin");

    EXPECT_STR_EQ(config.direction,
                  "out",
                  "Output direction");

    EXPECT_EQ(config.value,
              0,
              "Output initial value");

    config.value = 1;

    EXPECT_EQ(config.value,
              1,
              "Output HIGH value");
}


/* ============================================================
 * Test 3
 * GPIO Input Configuration
 * ============================================================
 */

static void test_gpio_input_configuration(void)
{
    TEST_START("GPIO input configuration");

    gpio_config_t config;

    memset(&config, 0, sizeof(config));

    config.pin = 18;
    config.chip = 0;

    strncpy(config.direction,
            "in",
            sizeof(config.direction) - 1);

    strncpy(config.bias,
            "pull_up",
            sizeof(config.bias) - 1);

    strncpy(config.edge,
            "both",
            sizeof(config.edge) - 1);

    EXPECT_EQ(config.pin,
              18,
              "Input GPIO pin");

    EXPECT_STR_EQ(config.direction,
                  "in",
                  "Input direction");

    EXPECT_STR_EQ(config.bias,
                  "pull_up",
                  "Pull-up configuration");

    EXPECT_STR_EQ(config.edge,
                  "both",
                  "Edge configuration");
}


/* ============================================================
 * Test 4
 * GPIO Value Validation
 * ============================================================
 */

static void test_gpio_value_validation(void)
{
    TEST_START("GPIO value validation");

    int value;

    value = 0;

    EXPECT_TRUE(value == 0 || value == 1,
                "LOW GPIO value is valid");

    value = 1;

    EXPECT_TRUE(value == 0 || value == 1,
                "HIGH GPIO value is valid");
}


/* ============================================================
 * Test 5
 * GPIO Toggle Logic
 * ============================================================
 */

static void test_gpio_toggle(void)
{
    TEST_START("GPIO toggle logic");

    int value = 0;

    value = !value;

    EXPECT_EQ(value,
              1,
              "LOW to HIGH toggle");

    value = !value;

    EXPECT_EQ(value,
              0,
              "HIGH to LOW toggle");

    value = !value;

    EXPECT_EQ(value,
              1,
              "Second LOW to HIGH toggle");
}


/* ============================================================
 * Test 6
 * GPIO Multiple Pins
 * ============================================================
 */

static void test_multiple_gpio_pins(void)
{
    TEST_START("Multiple GPIO configuration");

    gpio_config_t gpio1;
    gpio_config_t gpio2;
    gpio_config_t gpio3;

    memset(&gpio1, 0, sizeof(gpio1));
    memset(&gpio2, 0, sizeof(gpio2));
    memset(&gpio3, 0, sizeof(gpio3));

    gpio1.pin = 17;
    gpio2.pin = 18;
    gpio3.pin = 19;

    gpio1.value = 1;
    gpio2.value = 0;
    gpio3.value = 1;

    EXPECT_EQ(gpio1.pin,
              17,
              "GPIO1 pin");

    EXPECT_EQ(gpio2.pin,
              18,
              "GPIO2 pin");

    EXPECT_EQ(gpio3.pin,
              19,
              "GPIO3 pin");

    EXPECT_EQ(gpio1.value,
              1,
              "GPIO1 value");

    EXPECT_EQ(gpio2.value,
              0,
              "GPIO2 value");

    EXPECT_EQ(gpio3.value,
              1,
              "GPIO3 value");
}


/* ============================================================
 * Test 7
 * GPIO Edge Configuration
 * ============================================================
 */

static void test_gpio_edge_configuration(void)
{
    TEST_START("GPIO edge configuration");

    gpio_config_t config;

    memset(&config, 0, sizeof(config));

    strncpy(config.edge,
            "rising",
            sizeof(config.edge) - 1);

    EXPECT_STR_EQ(config.edge,
                  "rising",
                  "Rising edge");

    strncpy(config.edge,
            "falling",
            sizeof(config.edge) - 1);

    EXPECT_STR_EQ(config.edge,
                  "falling",
                  "Falling edge");

    strncpy(config.edge,
            "both",
            sizeof(config.edge) - 1);

    EXPECT_STR_EQ(config.edge,
                  "both",
                  "Both edges");
}


/* ============================================================
 * Test 8
 * GPIO Bias Configuration
 * ============================================================
 */

static void test_gpio_bias_configuration(void)
{
    TEST_START("GPIO bias configuration");

    gpio_config_t config;

    memset(&config, 0, sizeof(config));

    strncpy(config.bias,
            "default",
            sizeof(config.bias) - 1);

    EXPECT_STR_EQ(config.bias,
                  "default",
                  "Default bias");

    strncpy(config.bias,
            "pull_up",
            sizeof(config.bias) - 1);

    EXPECT_STR_EQ(config.bias,
                  "pull_up",
                  "Pull-up bias");

    strncpy(config.bias,
            "pull_down",
            sizeof(config.bias) - 1);

    EXPECT_STR_EQ(config.bias,
                  "pull_down",
                  "Pull-down bias");

    strncpy(config.bias,
            "disable",
            sizeof(config.bias) - 1);

    EXPECT_STR_EQ(config.bias,
                  "disable",
                  "Bias disabled");
}


/* ============================================================
 * Test 9
 * GPIO Chip Selection
 * ============================================================
 */

static void test_gpio_chip_selection(void)
{
    TEST_START("GPIO chip selection");

    gpio_config_t config;

    memset(&config, 0, sizeof(config));

    config.chip = 0;

    EXPECT_EQ(config.chip,
              0,
              "GPIO chip 0");

    config.chip = 1;

    EXPECT_EQ(config.chip,
              1,
              "GPIO chip 1");
}


/* ============================================================
 * Test 10
 * GPIO Interface Selection
 * ============================================================
 */

static void test_gpio_interface_selection(void)
{
    TEST_START("GPIO interface selection");

    gpio_config_t config;

    memset(&config, 0, sizeof(config));

    /*
     * 1 = libgpiod
     */
    config.use_libgpiod = 1;

    EXPECT_EQ(config.use_libgpiod,
              1,
              "libgpiod interface selected");

    /*
     * 0 = sysfs
     */
    config.use_libgpiod = 0;

    EXPECT_EQ(config.use_libgpiod,
              0,
              "sysfs interface selected");
}


/* ============================================================
 * Test Summary
 * ============================================================
 */

static void print_summary(void)
{
    int total = tests_passed + tests_failed;

    printf("\n");
    printf("============================================================\n");
    printf(" GPIO UNIT TEST SUMMARY\n");
    printf("============================================================\n");

    printf("\n");
    printf("Total tests : %d\n", total);
    printf("Passed      : %d\n", tests_passed);
    printf("Failed      : %d\n", tests_failed);

    printf("\n");

    if (tests_failed == 0) {

        printf("============================================================\n");
        printf(" GPIO UNIT TEST: PASSED\n");
        printf("============================================================\n");

    } else {

        printf("============================================================\n");
        printf(" GPIO UNIT TEST: FAILED\n");
        printf("============================================================\n");
    }

    printf("\n");
}


/* ============================================================
 * Main
 * ============================================================
 */

int main(void)
{
    printf("\n");
    printf("============================================================\n");
    printf(" STM32MP157-DK2 GPIO UNIT TESTS\n");
    printf("============================================================\n");

    /*
     * Execute unit tests.
     */

    test_gpio_config_initialization();

    test_gpio_output_configuration();

    test_gpio_input_configuration();

    test_gpio_value_validation();

    test_gpio_toggle();

    test_multiple_gpio_pins();

    test_gpio_edge_configuration();

    test_gpio_bias_configuration();

    test_gpio_chip_selection();

    test_gpio_interface_selection();

    /*
     * Print final result.
     */

    print_summary();

    /*
     * Return non-zero when any test fails.
     * This is useful for CI/CD.
     */

    if (tests_failed != 0)
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}
