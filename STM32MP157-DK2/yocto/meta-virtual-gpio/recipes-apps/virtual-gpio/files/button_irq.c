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
    gpio_gpiod_handle_t button;

    int chip = 0;
    int line = 0;

    int event;

    if (argc >= 2)
        line = atoi(argv[1]);

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    printf("========================================\n");
    printf(" STM32MP157-DK2 GPIO Interrupt Test\n");
    printf("========================================\n");

    printf("GPIO chip : %d\n", chip);
    printf("GPIO line : %d\n", line);

    if (gpio_gpiod_open(&button,
                        chip,
                        line) < 0)
        return EXIT_FAILURE;

    /*
     * Request both rising and falling edges.
     */
    if (gpio_gpiod_wait_event(&button,
                              "both",
                              1000) < 0) {

        gpio_gpiod_close(&button);
        return EXIT_FAILURE;
    }

    printf("Waiting for GPIO events...\n");
    printf("Press Ctrl+C to stop.\n");

    while (running) {

        event = gpio_gpiod_wait_event(
            &button,
            "both",
            1000);

        if (event < 0)
            break;

        if (event == 1) {
            printf("Interrupt: RISING EDGE\n");
            fflush(stdout);
        }
        else if (event == 2) {
            printf("Interrupt: FALLING EDGE\n");
            fflush(stdout);
        }
    }

    gpio_gpiod_close(&button);

    printf("\nInterrupt test stopped.\n");

    return EXIT_SUCCESS;
}
