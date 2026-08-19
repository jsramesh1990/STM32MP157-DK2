/*
 * gpio_toggle.c
 *
 * STM32MP157-DK2
 * Linux GPIO Toggle Example
 *
 * Demonstrates:
 *   - GPIO export
 *   - GPIO direction configuration
 *   - GPIO read
 *   - GPIO write
 *   - GPIO toggle operation
 *
 * Usage:
 *
 *   ./gpio_toggle
 *
 * Or:
 *
 *   ./gpio_toggle 13
 *
 * NOTE:
 * GPIO number is board/device-tree dependent.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#define GPIO_SYSFS "/sys/class/gpio"

#define DEFAULT_GPIO 13

#define TOGGLE_COUNT 20
#define TOGGLE_DELAY_US 500000

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

static int read_file(const char *path, char *buffer, size_t size)
{
    FILE *fp;

    fp = fopen(path, "r");

    if (fp == NULL) {
        fprintf(stderr,
                "ERROR: Cannot open %s: %s\n",
                path,
                strerror(errno));
        return -1;
    }

    if (fgets(buffer, size, fp) == NULL) {
        fprintf(stderr,
                "ERROR: Cannot read %s\n",
                path);

        fclose(fp);
        return -1;
    }

    fclose(fp);

    return 0;
}

static int gpio_export(int gpio)
{
    char gpio_str[16];

    snprintf(gpio_str,
             sizeof(gpio_str),
             "%d",
             gpio);

    /*
     * Export GPIO.
     */
    if (write_file(GPIO_SYSFS "/export", gpio_str) < 0) {
        /*
         * GPIO may already be exported.
         * Continue because it may already be usable.
         */
        printf("GPIO%d may already be exported.\n", gpio);
    }

    /*
     * Allow kernel/sysfs to create gpio directory.
     */
    usleep(100000);

    return 0;
}

static int gpio_unexport(int gpio)
{
    char gpio_str[16];

    snprintf(gpio_str,
             sizeof(gpio_str),
             "%d",
             gpio);

    return write_file(GPIO_SYSFS "/unexport",
                      gpio_str);
}

static int gpio_set_direction(int gpio,
                              const char *direction)
{
    char path[128];

    snprintf(path,
             sizeof(path),
             GPIO_SYSFS "/gpio%d/direction",
             gpio);

    return write_file(path, direction);
}

static int gpio_set_value(int gpio,
                          int value)
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

static int gpio_get_value(int gpio)
{
    char path[128];
    char buffer[16];

    snprintf(path,
             sizeof(path),
             GPIO_SYSFS "/gpio%d/value",
             gpio);

    if (read_file(path,
                  buffer,
                  sizeof(buffer)) < 0) {
        return -1;
    }

    return atoi(buffer);
}

static int gpio_toggle(int gpio)
{
    int current_value;
    int new_value;

    /*
     * Read current GPIO state.
     */
    current_value = gpio_get_value(gpio);

    if (current_value < 0) {
        fprintf(stderr,
                "Failed to read GPIO%d\n",
                gpio);

        return -1;
    }

    /*
     * Toggle:
     *
     * 0 -> 1
     * 1 -> 0
     */
    new_value = !current_value;

    /*
     * Write new state.
     */
    if (gpio_set_value(gpio,
                       new_value) < 0) {

        fprintf(stderr,
                "Failed to write GPIO%d\n",
                gpio);

        return -1;
    }

    printf("GPIO%d: %d -> %d\n",
           gpio,
           current_value,
           new_value);

    return new_value;
}

int main(int argc,
         char *argv[])
{
    int gpio = DEFAULT_GPIO;

    /*
     * Allow GPIO number from command line.
     */
    if (argc > 1) {

        gpio = atoi(argv[1]);

        if (gpio < 0) {
            fprintf(stderr,
                    "Invalid GPIO number\n");

            return EXIT_FAILURE;
        }
    }

    printf("========================================\n");
    printf(" STM32MP157-DK2 GPIO Toggle\n");
    printf("========================================\n");

    printf("GPIO: %d\n", gpio);

    /*
     * Step 1:
     * Export GPIO.
     */
    printf("[1] Exporting GPIO%d...\n",
           gpio);

    if (gpio_export(gpio) < 0) {
        fprintf(stderr,
                "Failed to export GPIO%d\n",
                gpio);

        return EXIT_FAILURE;
    }

    /*
     * Step 2:
     * Configure GPIO as output.
     */
    printf("[2] Configuring GPIO%d as output...\n",
           gpio);

    if (gpio_set_direction(gpio,
                           "out") < 0) {

        fprintf(stderr,
                "Failed to configure GPIO%d\n",
                gpio);

        gpio_unexport(gpio);

        return EXIT_FAILURE;
    }

    /*
     * Step 3:
     * Initialize GPIO LOW.
     */
    printf("[3] Initializing GPIO LOW\n");

    if (gpio_set_value(gpio, 0) < 0) {

        gpio_unexport(gpio);

        return EXIT_FAILURE;
    }

    /*
     * Step 4:
     * Toggle GPIO.
     */
    printf("[4] Starting toggle operation...\n");
    printf("Press Ctrl+C to stop.\n\n");

    for (int i = 0;
         i < TOGGLE_COUNT;
         i++) {

        int value;

        value = gpio_toggle(gpio);

        if (value < 0) {
            break;
        }

        usleep(TOGGLE_DELAY_US);
    }

    /*
     * Step 5:
     * Return GPIO to LOW.
     */
    printf("\n[5] Setting GPIO LOW...\n");

    gpio_set_value(gpio, 0);

    /*
     * Step 6:
     * Release GPIO.
     */
    printf("[6] Unexporting GPIO%d...\n",
           gpio);

    gpio_unexport(gpio);

    printf("GPIO toggle test completed.\n");

    return EXIT_SUCCESS;
}
