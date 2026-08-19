/*
 * STM32MP157-DK2
 * GPIO Button Example
 *
 * Purpose:
 *   Demonstrates GPIO input using libgpiod.
 *
 * Hardware:
 *   STM32MP157-DK2
 *
 * Flow:
 *   Button -> GPIO Controller -> Linux GPIO Driver
 *          -> /dev/gpiochipX -> libgpiod -> button.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <gpiod.h>

#define GPIO_CHIP       "/dev/gpiochip0"
#define GPIO_LINE       13
#define CONSUMER        "stm32mp157-button"

static volatile sig_atomic_t running = 1;

static void signal_handler(int signal)
{
    (void)signal;
    running = 0;
}

int main(void)
{
    struct gpiod_chip *chip = NULL;
    struct gpiod_line *line = NULL;

    int value;
    int last_value = -1;

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    printf("========================================\n");
    printf(" STM32MP157-DK2 GPIO Button Test\n");
    printf("========================================\n");
    printf("GPIO Chip : %s\n", GPIO_CHIP);
    printf("GPIO Line : %d\n", GPIO_LINE);
    printf("Press Ctrl+C to exit.\n\n");

    /*
     * Open GPIO chip.
     */
    chip = gpiod_chip_open(GPIO_CHIP);

    if (!chip) {
        fprintf(stderr,
                "ERROR: Cannot open GPIO chip %s: %s\n",
                GPIO_CHIP,
                strerror(errno));
        return EXIT_FAILURE;
    }

    /*
     * Get GPIO line.
     */
    line = gpiod_chip_get_line(chip, GPIO_LINE);

    if (!line) {
        fprintf(stderr,
                "ERROR: Cannot get GPIO line %d: %s\n",
                GPIO_LINE,
                strerror(errno));

        gpiod_chip_close(chip);
        return EXIT_FAILURE;
    }

    /*
     * Request GPIO as input.
     *
     * The actual pull-up/pull-down configuration should
     * normally be handled through Device Tree / pinctrl.
     */
    if (gpiod_line_request_input(line, CONSUMER) < 0) {
        fprintf(stderr,
                "ERROR: Cannot request GPIO%d as input: %s\n",
                GPIO_LINE,
                strerror(errno));

        gpiod_chip_close(chip);
        return EXIT_FAILURE;
    }

    printf("GPIO%d configured as INPUT\n", GPIO_LINE);
    printf("Waiting for button state changes...\n\n");

    while (running) {

        /*
         * Read GPIO value.
         */
        value = gpiod_line_get_value(line);

        if (value < 0) {
            fprintf(stderr,
                    "ERROR: Failed to read GPIO%d: %s\n",
                    GPIO_LINE,
                    strerror(errno));
            break;
        }

        /*
         * Print only when the state changes.
         */
        if (value != last_value) {

            if (value == 0) {
                printf("GPIO%d = 0 -> BUTTON PRESSED\n",
                       GPIO_LINE);
            } else {
                printf("GPIO%d = 1 -> BUTTON RELEASED\n",
                       GPIO_LINE);
            }

            last_value = value;
        }

        /*
         * Small polling delay.
         *
         * For interrupt-based operation,
         * use button_irq.c instead.
         */
        usleep(100000);
    }

    printf("\nStopping button application...\n");

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
