#include "gpio_common.h"
#include "gpio_libgpiod.h"

#include <signal.h>


static volatile sig_atomic_t running = 1;


static void signal_handler(int sig)
{
    (void)sig;
    running = 0;
}


int main(int argc, char *argv[])
{
    gpio_gpiod_handle_t gpio;

    int chip = 0;
    int line = 0;
    int delay_ms = 500;

    int value = 0;
    int count = 0;

    if (argc >= 2)
        line = atoi(argv[1]);

    if (argc >= 3)
        delay_ms = atoi(argv[2]);

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    printf("========================================\n");
    printf(" STM32MP157 GPIO Toggle\n");
    printf("========================================\n");

    printf("GPIO chip : %d\n", chip);
    printf("GPIO line : %d\n", line);
    printf("Delay     : %d ms\n", delay_ms);

    if (gpio_gpiod_open(&gpio,
                        chip,
                        line) < 0) {
        fprintf(stderr,
                "Failed to open GPIO\n");
        return EXIT_FAILURE;
    }

    if (gpio_gpiod_request_output(
            &gpio,
            "gpio-toggle",
            0) < 0) {

        gpio_gpiod_close(&gpio);
        return EXIT_FAILURE;
    }

    printf("Toggle started.\n");
    printf("Press Ctrl+C to stop.\n");

    while (running) {

        value = !value;

        if (gpio_gpiod_write(&gpio,
                             value) < 0) {
            break;
        }

        count++;

        printf("Toggle %d -> GPIO%d = %d\n",
               count,
               line,
               value);

        fflush(stdout);

        gpio_sleep_ms(delay_ms);
    }

    gpio_gpiod_write(&gpio, 0);

    gpio_gpiod_close(&gpio);

    printf("\nGPIO toggle stopped.\n");

    return EXIT_SUCCESS;
}
