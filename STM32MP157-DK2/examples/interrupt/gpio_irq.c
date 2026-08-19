/*
 * gpio_irq.c
 *
 * STM32MP157-DK2
 * Generic GPIO Interrupt Test
 *
 * Description:
 *   Monitors a GPIO line using the Linux GPIO subsystem
 *   through libgpiod.
 *
 * Build:
 *   make
 *
 * Run:
 *   sudo ./gpio_irq /dev/gpiochip0 14
 *
 * Example:
 *   sudo ./gpio_irq /dev/gpiochip0 14
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include <gpiod.h>

#define CONSUMER     "stm32mp157-gpio-irq"
#define DEFAULT_CHIP "/dev/gpiochip0"
#define DEFAULT_LINE 14

static volatile sig_atomic_t running = 1;

static void signal_handler(int signal)
{
    (void)signal;
    running = 0;
}

static void print_usage(const char *program)
{
    printf("\n");
    printf("STM32MP157-DK2 GPIO Interrupt Test\n");
    printf("----------------------------------\n\n");

    printf("Usage:\n");
    printf("  %s [gpiochip] [line]\n\n", program);

    printf("Example:\n");
    printf("  sudo %s /dev/gpiochip0 14\n\n", program);

    printf("Arguments:\n");
    printf("  gpiochip : GPIO chip device\n");
    printf("  line     : GPIO line offset\n\n");
}

static const char *event_to_string(
        enum gpiod_line_event_type type)
{
    switch (type) {

    case GPIOD_LINE_EVENT_RISING_EDGE:
        return "RISING EDGE";

    case GPIOD_LINE_EVENT_FALLING_EDGE:
        return "FALLING EDGE";

    default:
        return "UNKNOWN";
    }
}

int main(int argc, char *argv[])
{
    const char *chip_path = DEFAULT_CHIP;
    unsigned int line_offset = DEFAULT_LINE;

    struct gpiod_chip *chip = NULL;
    struct gpiod_line *line = NULL;
    struct gpiod_line_event event;

    struct timespec timeout;

    int ret;
    unsigned long event_count = 0;

    /*
     * Parse command line.
     */
    if (argc > 3) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    if (argc >= 2)
        chip_path = argv[1];

    if (argc == 3)
        line_offset =
            (unsigned int)strtoul(argv[2], NULL, 10);

    /*
     * Signal handling.
     */
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    printf("============================================\n");
    printf(" STM32MP157-DK2 GPIO Interrupt Test\n");
    printf("============================================\n");

    printf("GPIO chip : %s\n", chip_path);
    printf("GPIO line : %u\n", line_offset);
    printf("Consumer  : %s\n\n", CONSUMER);

    /*
     * Open GPIO chip.
     */
    chip = gpiod_chip_open(chip_path);

    if (!chip) {

        fprintf(stderr,
                "ERROR: Cannot open %s\n",
                chip_path);

        fprintf(stderr,
                "Reason: %s\n",
                strerror(errno));

        return EXIT_FAILURE;
    }

    /*
     * Get GPIO line.
     */
    line = gpiod_chip_get_line(chip, line_offset);

    if (!line) {

        fprintf(stderr,
                "ERROR: Cannot access GPIO line %u\n",
                line_offset);

        gpiod_chip_close(chip);

        return EXIT_FAILURE;
    }

    /*
     * Configure GPIO as input and enable
     * both rising and falling edge detection.
     */
    ret = gpiod_line_request_both_edges_events_flags(
        line,
        CONSUMER,
        GPIOD_LINE_REQUEST_FLAG_BIAS_PULL_UP
    );

    if (ret < 0) {

        fprintf(stderr,
                "ERROR: GPIO request failed\n");

        fprintf(stderr,
                "Reason: %s\n",
                strerror(errno));

        gpiod_chip_close(chip);

        return EXIT_FAILURE;
    }

    printf("GPIO configured successfully.\n");
    printf("Direction : INPUT\n");
    printf("Edge      : RISING + FALLING\n");
    printf("Bias      : PULL-UP\n\n");

    printf("Waiting for GPIO events...\n");
    printf("Press Ctrl+C to stop.\n\n");

    /*
     * Interrupt/event monitoring loop.
     */
    while (running) {

        /*
         * Wait one second for GPIO event.
         */
        timeout.tv_sec = 1;
        timeout.tv_nsec = 0;

        ret = gpiod_line_event_wait(
            line,
            &timeout
        );

        if (ret < 0) {

            fprintf(stderr,
                    "ERROR: Event wait failed: %s\n",
                    strerror(errno));

            break;
        }

        /*
         * No event.
         */
        if (ret == 0)
            continue;

        /*
         * Read event from GPIO line.
         */
        ret = gpiod_line_event_read(
            line,
            &event
        );

        if (ret < 0) {

            fprintf(stderr,
                    "ERROR: Event read failed: %s\n",
                    strerror(errno));

            break;
        }

        event_count++;

        printf("GPIO EVENT #%lu\n",
               event_count);

        printf("  Line      : %u\n",
               line_offset);

        printf("  Type      : %s\n",
               event_to_string(event.event_type));

        printf("  Timestamp : %ld.%09ld sec\n",
               event.ts.tv_sec,
               event.ts.tv_nsec);

        printf("--------------------------------------------\n");

        fflush(stdout);
    }

    printf("\n");
    printf("============================================\n");
    printf(" GPIO Interrupt Test Stopping\n");
    printf("============================================\n");

    printf("Total events detected: %lu\n",
           event_count);

    /*
     * Release GPIO line.
     */
    gpiod_line_release(line);

    /*
     * Close GPIO chip.
     */
    gpiod_chip_close(chip);

    printf("GPIO resources released.\n");

    return EXIT_SUCCESS;
}
