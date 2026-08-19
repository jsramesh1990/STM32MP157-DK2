/*
 * led_blink.c
 *
 * STM32MP157-DK2
 * Linux GPIO LED Blink Example
 *
 * Demonstrates:
 *   - GPIO export
 *   - GPIO direction configuration
 *   - GPIO value control
 *   - LED blinking
 *
 * NOTE:
 * GPIO numbers are board/device-tree dependent.
 * Update LED_GPIO according to the GPIO exposed by your STM32MP157-DK2
 * Linux device tree.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#define GPIO_SYSFS "/sys/class/gpio"

/*
 * Change this GPIO number according to your STM32MP157-DK2
 * device-tree GPIO configuration.
 */
#define LED_GPIO 13

#define BLINK_COUNT 20
#define BLINK_DELAY_US 500000

static int write_file(const char *path, const char *value)
{
    FILE *fp;

    fp = fopen(path, "w");

    if (fp == NULL) {
        fprintf(stderr,
                "ERROR: Cannot open %s: %s\n",
                path,
                strerror(errno));
        return -1;
    }

    if (fprintf(fp, "%s", value) < 0) {
        fprintf(stderr,
                "ERROR: Cannot write %s: %s\n",
                path,
                strerror(errno));
        fclose(fp);
        return -1;
    }

    fclose(fp);
    return 0;
}

static int gpio_export(int gpio)
{
    char path[128];
    char gpio_str[16];

    snprintf(path,
             sizeof(path),
             GPIO_SYSFS "/export");

    snprintf(gpio_str,
             sizeof(gpio_str),
             "%d",
             gpio);

    /*
     * GPIO may already be exported.
     * In that case we continue.
     */
    if (write_file(path, gpio_str) < 0) {
        if (errno != EBUSY) {
            /*
             * Do not immediately fail because some kernels
             * return an error when the GPIO is already exported.
             */
        }
    }

    return 0;
}

static int gpio_unexport(int gpio)
{
    char path[128];
    char gpio_str[16];

    snprintf(path,
             sizeof(path),
             GPIO_SYSFS "/unexport");

    snprintf(gpio_str,
             sizeof(gpio_str),
             "%d",
             gpio);

    return write_file(path, gpio_str);
}

static int gpio_set_direction(int gpio, const char *direction)
{
    char path[128];

    snprintf(path,
             sizeof(path),
             GPIO_SYSFS "/gpio%d/direction",
             gpio);

    return write_file(path, direction);
}

static int gpio_set_value(int gpio, int value)
{
    char path[128];
    char value_str[4];

    snprintf(path,
             sizeof(path),
             GPIO_SYSFS "/gpio%d/value",
             gpio);

    snprintf(value_str,
             sizeof(value_str),
             "%d",
             value ? 1 : 0);

    return write_file(path, value_str);
}

int main(void)
{
    printf("========================================\n");
    printf(" STM32MP157-DK2 GPIO LED Blink\n");
    printf("========================================\n");

    printf("GPIO: %d\n", LED_GPIO);

    /*
     * Step 1:
     * Export GPIO.
     */
    printf("[1] Exporting GPIO%d...\n", LED_GPIO);

    if (gpio_export(LED_GPIO) < 0) {
        fprintf(stderr, "Failed to export GPIO%d\n", LED_GPIO);
        return EXIT_FAILURE;
    }

    /*
     * Give sysfs time to create gpio directory.
     */
    usleep(100000);

    /*
     * Step 2:
     * Configure GPIO as output.
     */
    printf("[2] Configuring GPIO%d as output...\n", LED_GPIO);

    if (gpio_set_direction(LED_GPIO, "out") < 0) {
        fprintf(stderr,
                "Failed to configure GPIO%d as output\n",
                LED_GPIO);

        gpio_unexport(LED_GPIO);
        return EXIT_FAILURE;
    }

    /*
     * Step 3:
     * Initial LED state = OFF.
     */
    printf("[3] Setting initial LED state OFF\n");

    if (gpio_set_value(LED_GPIO, 0) < 0) {
        fprintf(stderr,
                "Failed to set GPIO%d\n",
                LED_GPIO);

        gpio_unexport(LED_GPIO);
        return EXIT_FAILURE;
    }

    /*
     * Step 4:
     * Blink LED.
     */
    printf("[4] Starting LED blink...\n");

    for (int i = 0; i < BLINK_COUNT; i++) {

        printf("Cycle %02d: LED ON\n", i + 1);

        if (gpio_set_value(LED_GPIO, 1) < 0) {
            fprintf(stderr, "Failed to turn LED ON\n");
            break;
        }

        usleep(BLINK_DELAY_US);

        printf("Cycle %02d: LED OFF\n", i + 1);

        if (gpio_set_value(LED_GPIO, 0) < 0) {
            fprintf(stderr, "Failed to turn LED OFF\n");
            break;
        }

        usleep(BLINK_DELAY_US);
    }

    /*
     * Step 5:
     * Make sure LED is OFF.
     */
    gpio_set_value(LED_GPIO, 0);

    /*
     * Step 6:
     * Release GPIO.
     */
    printf("[5] Unexporting GPIO%d...\n", LED_GPIO);

    gpio_unexport(LED_GPIO);

    printf("LED blink test completed.\n");

    return EXIT_SUCCESS;
}
