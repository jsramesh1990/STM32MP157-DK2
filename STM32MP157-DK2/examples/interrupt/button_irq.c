/*
 * button_irq.c
 *
 * STM32MP157-DK2
 * GPIO Button Interrupt Example
 *
 * Description:
 *   Monitors a GPIO input using libgpiod and detects
 *   rising/falling edge events.
 *
 * Build:
 *   make
 *
 * Run:
 *   sudo ./button_irq /dev/gpiochip0 14
 *
 * Arguments:
 *   argv[1] = GPIO chip device
 *   argv[2] = GPIO line offset
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/select.h>

#include <gpiod.h>

#define CONSUMER        "stm32mp157-button"
#define DEFAULT_CHIP    "/dev/gpiochip0"
#define DEFAULT_LINE    14

static volatile sig_atomic_t running = 1;

static void signal_handler(int signum)
{
    (void)signum;
    running = 0;
}

static void print_usage(const char *program)
{
    printf("\nUsage:\n");
    printf("  %s [gpiochip] [line]\n\n", program);

    printf("Example:\n");
    printf("  sudo %s /dev/gpiochip0 14\n\n", program);

    printf("Arguments:\n");
    printf("  gpiochip   GPIO chip device, default: %s\n", DEFAULT_CHIP);
    printf("  line       GPIO line offset, default: %d\n", DEFAULT_LINE);
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

    if (argc > 3) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    if (argc >= 2)
        chip_path = argv[1];

    if (argc == 3)
        line_offset = (unsigned int)strtoul(argv[2], NULL, 10);

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    printf("========================================\n");
    printf(" STM32MP157-DK2 Button Interrupt Test\n");
    printf("========================================\n");
    printf("GPIO Chip : %s\n", chip_path);
    printf("GPIO Line : %u\n", line_offset);
    printf("Consumer  : %s\n\n", CONSUMER);

    /*
     * Open GPIO chip.
     */
    chip = gpiod_chip_open(chip_path);

    if (!chip) {
        fprintf(stderr,
                "ERROR: Failed to open GPIO chip %s: %s\n",
                chip_path,
                strerror(errno));
        return EXIT_FAILURE;
    }

    /*
     * Get GPIO line.
     */
    line = gpiod_chip_get_line(chip, line_offset);

    if (!line) {
        fprintf(stderr,
                "ERROR: Failed to get GPIO line %u: %s\n",
                line_offset,
                strerror(errno));

        gpiod_chip_close(chip);
        return EXIT_FAILURE;
    }

    /*
     * Request input GPIO with both-edge event detection.
     *
     * Pull-up is normally useful for a button connected
     * between GPIO and GND.
     */
    ret = gpiod_line_request_both_edges_events_flags(
        line,
        CONSUMER,
        GPIOD_LINE_REQUEST_FLAG_BIAS_PULL_UP
    );

    if (ret < 0) {
        fprintf(stderr,
                "ERROR: Failed to request GPIO line %u: %s\n",
                line_offset,
                strerror(errno));

        gpiod_chip_close(chip);
        return EXIT_FAILURE;
    }

    printf("GPIO interrupt monitoring started.\n");
    printf("Press the button or generate an edge.\n");
    printf("Press Ctrl+C to exit.\n\n");

    while (running) {

        /*
         * Wait up to one second for an event.
         */
        timeout.tv_sec = 1;
        timeout.tv_nsec = 0;

        ret = gpiod_line_event_wait(line, &timeout);

        if (ret < 0) {
            fprintf(stderr,
                    "ERROR: Event wait failed: %s\n",
                    strerror(errno));
            break;
        }

        if (ret == 0) {
            /*
             * Timeout.
             */
            continue;
        }

        /*
         * Read the GPIO event.
         */
        ret = gpiod_line_event_read(line, &event);

        if (ret < 0) {
            fprintf(stderr,
                    "ERROR: Failed to read GPIO event: %s\n",
                    strerror(errno));
            break;
        }

        if (event.event_type ==
            GPIOD_LINE_EVENT_RISING_EDGE) {

            printf("[GPIO IRQ] Rising edge detected\n");
            printf("            Button RELEASED\n");

        } else if (event.event_type ==
                   GPIOD_LINE_EVENT_FALLING_EDGE) {

            printf("[GPIO IRQ] Falling edge detected\n");
            printf("            Button PRESSED\n");
        }

        fflush(stdout);
    }

    printf("\nStopping GPIO interrupt monitoring...\n");

    /*
     * Release GPIO line.
     */
    gpiod_line_release(line);

    /*
     * Close GPIO chip.
     */
    gpiod_chip_close(chip);

    printf("GPIO interrupt test completed.\n");

    return EXIT_SUCCESS;
}
