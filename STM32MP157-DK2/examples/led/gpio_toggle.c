/*
 * gpio_toggle.c
 *
 * Simple GPIO toggle example for STM32MP157-DK2.
 *
 * This example uses the project's common GPIO API.
 * The GPIO number can be changed according to the
 * GPIO defined in the Device Tree / board configuration.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include "../../application/include/gpio_common.h"

#define GPIO_PIN        13
#define TOGGLE_DELAY_US 500000

static volatile int running = 1;

/*
 * Handle Ctrl+C
 */
static void signal_handler(int sig)
{
    (void)sig;
    running = 0;
}

int main(void)
{
    gpio_config_t gpio;
    int value = 0;

    signal(SIGINT, signal_handler);

    printf("========================================\n");
    printf(" STM32MP157-DK2 GPIO Toggle Example\n");
    printf("========================================\n");
    printf("GPIO: %d\n", GPIO_PIN);
    printf("Press Ctrl+C to stop\n\n");

    /*
     * Initialize GPIO configuration
     */
    gpio.pin = GPIO_PIN;
    gpio.chip = 0;
    gpio.value = 0;
    gpio.use_libgpiod = 1;

    snprintf(gpio.direction, sizeof(gpio.direction), "out");
    snprintf(gpio.edge, sizeof(gpio.edge), "none");
    snprintf(gpio.bias, sizeof(gpio.bias), "default");

    /*
     * Initialize GPIO
     */
    if (gpio_init(&gpio) < 0) {
        fprintf(stderr, "ERROR: GPIO initialization failed\n");
        return EXIT_FAILURE;
    }

    /*
     * Toggle GPIO continuously
     */
    while (running) {

        value = !value;

        if (gpio_write(&gpio, value) < 0) {
            fprintf(stderr, "ERROR: Failed to write GPIO\n");
            break;
        }

        printf("GPIO%d -> %s\n",
               GPIO_PIN,
               value ? "HIGH" : "LOW");

        usleep(TOGGLE_DELAY_US);
    }

    /*
     * Set GPIO LOW before exit
     */
    gpio_write(&gpio, 0);

    /*
     * Cleanup
     */
    gpio_cleanup(&gpio);

    printf("\nGPIO toggle stopped\n");

    return EXIT_SUCCESS;
}
