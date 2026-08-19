/*
 * button.c
 *
 * STM32MP157-DK2 GPIO Button Example
 *
 * Demonstrates:
 *   - Opening GPIO chip
 *   - Requesting GPIO as input
 *   - Reading GPIO state
 *   - Detecting button press
 *   - Releasing GPIO resources
 *
 * Usage:
 *
 *   ./gpio-button <chip> <line>
 *
 * Example:
 *
 *   ./gpio-button 0 10
 *
 * Hardware:
 *
 *   GPIO ---- Button ---- GND
 *
 * The GPIO should have an appropriate pull-up configuration.
 */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <gpiod.h>

#include "gpio_common.h"


/* ------------------------------------------------------------
 * Global running flag
 * ------------------------------------------------------------ */

static volatile sig_atomic_t running = 1;


/* ------------------------------------------------------------
 * Signal handler
 * ------------------------------------------------------------ */

static void signal_handler(int signal)
{
    (void)signal;

    running = 0;
}


/* ------------------------------------------------------------
 * Print usage
 * ------------------------------------------------------------ */

static void print_usage(const char *program)
{
    printf("\n");
    printf("STM32MP157-DK2 GPIO Button Test\n");
    printf("\n");

    printf("Usage:\n");
    printf("  %s <chip> <line>\n",
           program);

    printf("\n");

    printf("Example:\n");
    printf("  %s 0 10\n",
           program);

    printf("\n");

    printf("Arguments:\n");
    printf("  chip    GPIO chip number\n");
    printf("  line    GPIO line number\n");

    printf("\n");
}


/* ------------------------------------------------------------
 * Main
 * ------------------------------------------------------------ */

int main(int argc, char *argv[])
{
    int chip_number;
    int line_number;

    struct gpiod_chip *chip = NULL;
    struct gpiod_line *button = NULL;

    int last_value = -1;


    /* --------------------------------------------------------
     * Argument validation
     * -------------------------------------------------------- */

    if (argc != 3) {

        print_usage(argv[0]);

        return EXIT_FAILURE;
    }


    chip_number = atoi(argv[1]);
    line_number = atoi(argv[2]);


    if (chip_number < 0 || line_number < 0) {

        fprintf(stderr,
                "[BUTTON][ERROR] Invalid GPIO parameters\n");

        return EXIT_FAILURE;
    }


    /* --------------------------------------------------------
     * Install signal handlers
     * -------------------------------------------------------- */

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);


    printf("\n");
    printf("============================================\n");
    printf("       STM32MP157-DK2 BUTTON TEST\n");
    printf("============================================\n");

    printf("GPIO Chip : %d\n",
           chip_number);

    printf("GPIO Line : %d\n",
           line_number);

    printf("============================================\n");
    printf("\n");


    /* --------------------------------------------------------
     * Open GPIO chip
     * -------------------------------------------------------- */

    chip = gpio_chip_open(chip_number);

    if (chip == NULL) {

        fprintf(stderr,
                "[BUTTON][ERROR] Cannot open GPIO chip\n");

        return EXIT_FAILURE;
    }


    /* --------------------------------------------------------
     * Request GPIO as input
     * -------------------------------------------------------- */

    button =
        gpio_request_input(
            chip,
            line_number,
            "stm32mp157-button");

    if (button == NULL) {

        fprintf(stderr,
                "[BUTTON][ERROR] Cannot request GPIO input\n");

        gpio_chip_close(chip);

        return EXIT_FAILURE;
    }


    printf("\n");
    printf("[BUTTON] Button monitoring started\n");
    printf("[BUTTON] Press the physical button\n");
    printf("[BUTTON] Press Ctrl+C to exit\n");
    printf("\n");


    /* --------------------------------------------------------
     * Button monitoring loop
     * -------------------------------------------------------- */

    while (running) {

        int value;

        value =
            gpio_get_value(button);

        if (value < 0) {

            fprintf(stderr,
                    "[BUTTON][ERROR] Failed to read button\n");

            break;
        }


        /* ----------------------------------------------------
         * Detect state change
         * ---------------------------------------------------- */

        if (value != last_value) {

            if (value == GPIO_LOW) {

                printf("[BUTTON] PRESSED\n");

            } else {

                printf("[BUTTON] RELEASED\n");
            }

            last_value = value;
        }


        /*
         * Small polling delay.
         *
         * For real interrupt-driven operation,
         * use button_irq.c.
         */

        usleep(10000);
    }


    /* --------------------------------------------------------
     * Cleanup
     * -------------------------------------------------------- */

    printf("\n");
    printf("[BUTTON] Stopping button monitor\n");

    gpio_line_release(button);

    gpio_chip_close(chip);

    printf("[BUTTON] Cleanup completed\n");

    return EXIT_SUCCESS;
}
