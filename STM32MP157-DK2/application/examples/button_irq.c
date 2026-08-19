/*
 * button_irq.c
 *
 * STM32MP157-DK2 GPIO Button Interrupt Example
 *
 * Demonstrates:
 *   - Linux GPIO character device
 *   - libgpiod
 *   - GPIO input
 *   - Pull-up configuration
 *   - Rising/Falling edge events
 *   - GPIO interrupt/event handling
 *   - Software debounce
 *
 * Usage:
 *   ./button_irq <gpiochip> <line>
 *
 * Example:
 *   ./button_irq /dev/gpiochip0 5
 *
 * IMPORTANT:
 *   GPIO chip and line must match the actual STM32MP157-DK2
 *   hardware configuration.
 */

#include <errno.h>
#include <gpiod.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define DEFAULT_GPIOCHIP "/dev/gpiochip0"
#define DEFAULT_LINE     5

#define DEBOUNCE_MS      50

static volatile sig_atomic_t running = 1;

/*
 * Signal handler
 */
static void signal_handler(int signal)
{
    (void)signal;
    running = 0;
}

/*
 * Get current monotonic time in milliseconds.
 */
static long long get_time_ms(void)
{
    struct timespec ts;

    clock_gettime(CLOCK_MONOTONIC, &ts);

    return ((long long)ts.tv_sec * 1000LL) +
           ((long long)ts.tv_nsec / 1000000LL);
}

/*
 * Print GPIO event.
 */
static void print_event(
    enum gpiod_edge_event_type event_type)
{
    switch (event_type) {

    case GPIOD_EDGE_EVENT_RISING_EDGE:
        printf("[GPIO IRQ] RISING EDGE\n");
        break;

    case GPIOD_EDGE_EVENT_FALLING_EDGE:
        printf("[GPIO IRQ] FALLING EDGE\n");
        break;

    default:
        printf("[GPIO IRQ] UNKNOWN EDGE\n");
        break;
    }

    fflush(stdout);
}

/*
 * Print usage.
 */
static void print_usage(const char *program)
{
    printf("\n");
    printf("STM32MP157-DK2 GPIO Interrupt Example\n");
    printf("--------------------------------------\n");
    printf("Usage:\n");
    printf("  %s [gpiochip] [line]\n", program);
    printf("\n");
    printf("Examples:\n");
    printf("  %s /dev/gpiochip0 5\n", program);
    printf("  %s /dev/gpiochip1 3\n", program);
    printf("\n");
}

int main(int argc, char *argv[])
{
    const char *chip_path = DEFAULT_GPIOCHIP;
    unsigned int line_offset = DEFAULT_LINE;

    struct gpiod_chip *chip = NULL;
    struct gpiod_line_settings *settings = NULL;
    struct gpiod_line_config *line_config = NULL;
    struct gpiod_line_request *request = NULL;
    struct gpiod_edge_event_buffer *event_buffer = NULL;

    int ret = EXIT_FAILURE;

    long long last_event_time = 0;

    /*
     * Register signal handlers.
     */
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    /*
     * Parse arguments.
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

        value = strtoul(
            argv[2],
            &endptr,
            10);

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
    printf(" STM32MP157-DK2 GPIO IRQ Example\n");
    printf("========================================\n");

    printf("GPIO Chip : %s\n", chip_path);
    printf("GPIO Line : %u\n", line_offset);
    printf("Debounce  : %d ms\n", DEBOUNCE_MS);

    printf("\n");

    /*
     * Open GPIO chip.
     */
    chip = gpiod_chip_open(chip_path);

    if (!chip) {

        fprintf(stderr,
                "Failed to open GPIO chip %s: %s\n",
                chip_path,
                strerror(errno));

        goto cleanup;
    }

    /*
     * Create GPIO line settings.
     */
    settings = gpiod_line_settings_new();

    if (!settings) {

        fprintf(stderr,
                "Failed to create GPIO line settings\n");

        goto cleanup;
    }

    /*
     * GPIO input.
     */
    ret = gpiod_line_settings_set_direction(
        settings,
        GPIOD_LINE_DIRECTION_INPUT);

    if (ret < 0) {

        fprintf(stderr,
                "Failed to configure GPIO input\n");

        goto cleanup;
    }

    /*
     * Enable internal pull-up.
     *
     * Button wiring:
     *
     *        3.3V
     *         |
     *       Pull-up
     *         |
     * GPIO ---+------ BUTTON ------ GND
     *
     * Released = HIGH
     * Pressed  = LOW
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
     * Enable both rising and falling edge events.
     *
     * Falling:
     *     Button pressed
     *
     * Rising:
     *     Button released
     */
    ret = gpiod_line_settings_set_edge_detection(
        settings,
        GPIOD_LINE_EDGE_BOTH);

    if (ret < 0) {

        fprintf(stderr,
                "Failed to configure GPIO edge detection\n");

        goto cleanup;
    }

    /*
     * Create GPIO line configuration.
     */
    line_config = gpiod_line_config_new();

    if (!line_config) {

        fprintf(stderr,
                "Failed to create GPIO line configuration\n");

        goto cleanup;
    }

    /*
     * Add line configuration.
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
     * Request GPIO line.
     */
    request = gpiod_chip_request_lines(
        chip,
        "stm32mp157-button-irq",
        line_config);

    if (!request) {

        fprintf(stderr,
                "Failed to request GPIO line: %s\n",
                strerror(errno));

        goto cleanup;
    }

    /*
     * Allocate event buffer.
     */
    event_buffer = gpiod_edge_event_buffer_new(16);

    if (!event_buffer) {

        fprintf(stderr,
                "Failed to allocate GPIO event buffer\n");

        goto cleanup;
    }

    printf("GPIO interrupt/event configuration successful.\n");
    printf("\n");
    printf("Waiting for button events...\n");
    printf("Press Ctrl+C to exit.\n");
    printf("\n");

    /*
     * Main event loop.
     */
    while (running) {

        int wait_ret;

        /*
         * Wait for GPIO edge event.
         *
         * -1 means wait indefinitely.
         */
        wait_ret = gpiod_line_request_wait_edge_events(
            request,
            -1);

        if (wait_ret < 0) {

            if (!running)
                break;

            fprintf(stderr,
                    "Error waiting for GPIO event: %s\n",
                    strerror(errno));

            break;
        }

        if (wait_ret == 0)
            continue;

        /*
         * Read pending GPIO events.
         */
        ret = gpiod_line_request_read_edge_events(
            request,
            event_buffer,
            16);

        if (ret < 0) {

            fprintf(stderr,
                    "Failed to read GPIO event: %s\n",
                    strerror(errno));

            break;
        }

        /*
         * Process every received event.
         */
        for (int i = 0; i < ret; i++) {

            struct gpiod_edge_event *event;

            long long current_time;

            enum gpiod_edge_event_type event_type;

            event = gpiod_edge_event_buffer_get_event(
                event_buffer,
                i);

            if (!event)
                continue;

            event_type =
                gpiod_edge_event_get_event_type(event);

            current_time = get_time_ms();

            /*
             * Software debounce.
             *
             * Mechanical buttons can generate
             * multiple transitions for one press.
             */
            if (last_event_time != 0 &&
                (current_time - last_event_time) <
                    DEBOUNCE_MS) {

                continue;
            }

            last_event_time = current_time;

            /*
             * Display event.
             */
            print_event(event_type);

            /*
             * Additional application processing.
             */
            switch (event_type) {

            case GPIOD_EDGE_EVENT_FALLING_EDGE:

                /*
                 * Active-low button pressed.
                 */
                printf("           BUTTON PRESSED\n");
                break;

            case GPIOD_EDGE_EVENT_RISING_EDGE:

                /*
                 * Active-low button released.
                 */
                printf("           BUTTON RELEASED\n");
                break;

            default:
                break;
            }

            printf("\n");
        }
    }

    printf("\n");
    printf("GPIO interrupt monitoring stopped.\n");

    ret = EXIT_SUCCESS;

cleanup:

    if (event_buffer)
        gpiod_edge_event_buffer_free(event_buffer);

    if (request)
        gpiod_line_request_release(request);

    if (line_config)
        gpiod_line_config_free(line_config);

    if (settings)
        gpiod_line_settings_free(settings);

    if (chip)
        gpiod_chip_close(chip);

    return ret;
}
