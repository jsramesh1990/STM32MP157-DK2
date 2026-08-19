/*
 * button.c
 *
 * STM32MP157-DK2 GPIO Button Example
 *
 * Demonstrates:
 *   - Linux GPIO character device
 *   - libgpiod
 *   - GPIO input
 *   - Pull-up configuration
 *   - Button polling
 *   - Software debouncing
 *
 * Usage:
 *   ./button <gpiochip> <line>
 *
 * Example:
 *   ./button /dev/gpiochip0 5
 *
 * IMPORTANT:
 *   The GPIO chip and line must match the actual GPIO
 *   connection/pin configuration of the STM32MP157-DK2.
 */

#include <errno.h>
#include <gpiod.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define DEFAULT_GPIOCHIP "/dev/gpiochip0"
#define DEFAULT_LINE     5

#define POLL_INTERVAL_US 10000
#define DEBOUNCE_COUNT   5

static volatile sig_atomic_t running = 1;

/*
 * SIGINT handler
 */
static void signal_handler(int signal)
{
    (void)signal;
    running = 0;
}

/*
 * Print usage information
 */
static void print_usage(const char *program)
{
    printf("\n");
    printf("STM32MP157-DK2 GPIO Button Example\n");
    printf("-----------------------------------\n");
    printf("Usage:\n");
    printf("  %s [gpiochip] [line]\n", program);
    printf("\n");
    printf("Examples:\n");
    printf("  %s /dev/gpiochip0 5\n", program);
    printf("  %s /dev/gpiochip1 3\n", program);
    printf("\n");
}

/*
 * Read GPIO value.
 *
 * For an active-low button:
 *
 *   0 -> Button pressed
 *   1 -> Button released
 */
static int read_button(struct gpiod_line_request *request,
                       unsigned int offset)
{
    int ret;
    enum gpiod_line_value value;

    ret = gpiod_line_request_get_value(request, offset, &value);

    if (ret < 0) {
        fprintf(stderr,
                "Failed to read GPIO line: %s\n",
                strerror(-ret));
        return -1;
    }

    if (value == GPIOD_LINE_VALUE_ACTIVE)
        return 1;

    if (value == GPIOD_LINE_VALUE_INACTIVE)
        return 0;

    return -1;
}

int main(int argc, char *argv[])
{
    const char *chip_path = DEFAULT_GPIOCHIP;
    unsigned int line_offset = DEFAULT_LINE;

    struct gpiod_chip *chip = NULL;
    struct gpiod_line_settings *settings = NULL;
    struct gpiod_line_config *line_config = NULL;
    struct gpiod_line_request *request = NULL;

    int ret;
    int previous_state;
    int stable_state;
    int candidate_state;
    int debounce_counter = 0;

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    /*
     * Parse command-line arguments
     */
    if (argc > 1) {
        if (strcmp(argv[1], "--help") == 0 ||
            strcmp(argv[1], "-h") == 0) {
            print_usage(argv[0]);
            return EXIT_SUCCESS;
        }

        chip_path = argv[1];
    }

    if (argc > 2) {
        char *endptr = NULL;
        unsigned long value;

        errno = 0;
        value = strtoul(argv[2], &endptr, 10);

        if (errno != 0 ||
            endptr == argv[2] ||
            *endptr != '\0') {
            fprintf(stderr,
                    "Invalid GPIO line: %s\n",
                    argv[2]);
            return EXIT_FAILURE;
        }

        line_offset = (unsigned int)value;
    }

    printf("\n");
    printf("========================================\n");
    printf(" STM32MP157-DK2 GPIO Button Example\n");
    printf("========================================\n");
    printf("GPIO Chip : %s\n", chip_path);
    printf("GPIO Line : %u\n", line_offset);
    printf("\n");

    /*
     * Open GPIO chip
     */
    chip = gpiod_chip_open(chip_path);

    if (!chip) {
        fprintf(stderr,
                "Failed to open GPIO chip %s: %s\n",
                chip_path,
                strerror(errno));
        return EXIT_FAILURE;
    }

    /*
     * Create line settings
     */
    settings = gpiod_line_settings_new();

    if (!settings) {
        fprintf(stderr,
                "Failed to create GPIO line settings\n");
        ret = EXIT_FAILURE;
        goto cleanup;
    }

    /*
     * Configure GPIO as input
     */
    ret = gpiod_line_settings_set_direction(
        settings,
        GPIOD_LINE_DIRECTION_INPUT);

    if (ret < 0) {
        fprintf(stderr,
                "Failed to configure GPIO direction\n");
        goto cleanup;
    }

    /*
     * Configure pull-up.
     *
     * Typical active-low button:
     *
     *       3.3V
     *        |
     *      Pull-up
     *        |
     * GPIO---+------ Button ------ GND
     *
     * Released = 1
     * Pressed  = 0
     */
    ret = gpiod_line_settings_set_bias(
        settings,
        GPIOD_LINE_BIAS_PULL_UP);

    if (ret < 0) {
        fprintf(stderr,
                "Failed to configure GPIO pull-up\n");
        goto cleanup;
    }

    /*
     * Create line configuration
     */
    line_config = gpiod_line_config_new();

    if (!line_config) {
        fprintf(stderr,
                "Failed to create GPIO line configuration\n");
        ret = -ENOMEM;
        goto cleanup;
    }

    /*
     * Add our GPIO line to the configuration
     */
    ret = gpiod_line_config_add_line_settings(
        line_config,
        &line_offset,
        1,
        settings);

    if (ret < 0) {
        fprintf(stderr,
                "Failed to add GPIO line configuration\n");
        goto cleanup;
    }

    /*
     * Request GPIO line from kernel
     */
    request = gpiod_chip_request_lines(
        chip,
        "stm32mp157-button",
        line_config);

    if (!request) {
        fprintf(stderr,
                "Failed to request GPIO line: %s\n",
                strerror(errno));
        ret = EXIT_FAILURE;
        goto cleanup;
    }

    printf("GPIO configured successfully.\n");
    printf("Button monitoring started.\n");
    printf("Press Ctrl+C to exit.\n\n");

    /*
     * Initial state
     */
    stable_state = read_button(request, line_offset);

    if (stable_state < 0) {
        ret = EXIT_FAILURE;
        goto cleanup;
    }

    previous_state = stable_state;
    candidate_state = stable_state;

    if (stable_state == 0)
        printf("Initial state: PRESSED\n");
    else
        printf("Initial state: RELEASED\n");

    /*
     * Main polling loop
     */
    while (running) {

        int current_state;

        current_state = read_button(
            request,
            line_offset);

        if (current_state < 0) {
            ret = EXIT_FAILURE;
            break;
        }

        /*
         * State changed from the currently stable state.
         */
        if (current_state != candidate_state) {
            candidate_state = current_state;
            debounce_counter = 0;
        } else {
            /*
             * Same state observed repeatedly.
             */
            if (debounce_counter < DEBOUNCE_COUNT)
                debounce_counter++;
        }

        /*
         * State remained stable long enough
         * to be considered a real button event.
         */
        if (debounce_counter >= DEBOUNCE_COUNT &&
            candidate_state != stable_state) {

            stable_state = candidate_state;

            if (stable_state == 0) {
                printf("[BUTTON] PRESSED\n");
            } else {
                printf("[BUTTON] RELEASED\n");
            }

            fflush(stdout);
        }

        previous_state = current_state;

        /*
         * 10 ms polling interval
         */
        usleep(POLL_INTERVAL_US);
    }

    printf("\nButton monitoring stopped.\n");

    ret = EXIT_SUCCESS;

cleanup:

    if (request)
        gpiod_line_request_release(request);

    if (line_config)
        gpiod_line_config_free(line_config);

    if (settings)
        gpiod_line_settings_free(settings);

    if (chip)
        gpiod_chip_close(chip);

    return ret == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
