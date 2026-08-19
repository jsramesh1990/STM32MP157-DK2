/*
 * led_blink.c
 *
 * LED blink example for STM32MP157-DK2.
 *
 * Demonstrates:
 *   - GPIO initialization
 *   - GPIO output configuration
 *   - LED ON/OFF control
 *   - Timing using usleep()
 *   - Proper GPIO cleanup
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include "../../application/include/gpio_common.h"

#define LED_GPIO        13
#define BLINK_DELAY_US  500000
#define BLINK_COUNT     10

static volatile int running = 1;

/*
 * Signal handler for Ctrl+C
 */
static void signal_handler(int sig)
{
    (void)sig;
    running = 0;
}

/*
 * Turn LED ON
 */
static int led_on(gpio_config_t *gpio)
{
    printf("LED -> ON\n");

    return gpio_write(gpio, 1);
}

/*
 * Turn LED OFF
 */
static int led_off(gpio_config_t *gpio)
{
    printf("LED -> OFF\n");

    return gpio_write(gpio, 0);
}

int main(void)
{
    gpio_config_t gpio;
    int i;

    signal(SIGINT, signal_handler);

    printf("========================================\n");
    printf(" STM32MP157-DK2 LED Blink Example\n");
    printf("========================================\n");

    printf("LED GPIO       : %d\n", LED_GPIO);
    printf("Blink count    : %d\n", BLINK_COUNT);
    printf("Blink interval : %d ms\n",
           BLINK_DELAY_US / 1000);

    printf("\nPress Ctrl+C to stop\n\n");

    /*
     * Configure GPIO
     */
    gpio.pin = LED_GPIO;
    gpio.chip = 0;
    gpio.value = 0;
    gpio.use_libgpiod = 1;

    snprintf(gpio.direction,
             sizeof(gpio.direction),
             "out");

    snprintf(gpio.edge,
             sizeof(gpio.edge),
             "none");

    snprintf(gpio.bias,
             sizeof(gpio.bias),
             "default");

    /*
     * Initialize GPIO
     */
    if (gpio_init(&gpio) < 0) {
        fprintf(stderr,
                "ERROR: Failed to initialize LED GPIO\n");

        return EXIT_FAILURE;
    }

    /*
     * Make sure LED starts OFF
     */
    if (led_off(&gpio) < 0) {
        fprintf(stderr,
                "ERROR: Failed to turn LED OFF\n");

        gpio_cleanup(&gpio);
        return EXIT_FAILURE;
    }

    /*
     * Blink LED
     */
    for (i = 0; i < BLINK_COUNT && running; i++) {

        printf("Blink cycle %d/%d\n",
               i + 1,
               BLINK_COUNT);

        /*
         * LED ON
         */
        if (led_on(&gpio) < 0) {
            fprintf(stderr,
                    "ERROR: Failed to turn LED ON\n");
            break;
        }

        usleep(BLINK_DELAY_US);

        /*
         * LED OFF
         */
        if (led_off(&gpio) < 0) {
            fprintf(stderr,
                    "ERROR: Failed to turn LED OFF\n");
            break;
        }

        usleep(BLINK_DELAY_US);
    }

    /*
     * Always leave LED OFF
     */
    gpio_write(&gpio, 0);

    /*
     * Release GPIO
     */
    gpio_cleanup(&gpio);

    printf("\nLED blink example completed\n");

    return EXIT_SUCCESS;
}
