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
    gpio_gpiod_handle_t led;

    int chip = 0;
    int line = 0;

    int delay_ms = 500;
    int cycles = 10;

    int i;

    if (argc >= 2)
        line = atoi(argv[1]);

    if (argc >= 3)
        delay_ms = atoi(argv[2]);

    if (argc >= 4)
        cycles = atoi(argv[3]);

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    printf("========================================\n");
    printf(" STM32MP157-DK2 LED Blink\n");
    printf("========================================\n");

    printf("GPIO chip : %d\n", chip);
    printf("GPIO line : %d\n", line);
    printf("Delay     : %d ms\n", delay_ms);
    printf("Cycles    : %d\n", cycles);

    if (gpio_gpiod_open(&led,
                        chip,
                        line) < 0)
        return EXIT_FAILURE;

    if (gpio_gpiod_request_output(
            &led,
            "led-blink",
            0) < 0) {

        gpio_gpiod_close(&led);
        return EXIT_FAILURE;
    }

    printf("Blinking LED...\n");

    for (i = 0;
         i < cycles && running;
         i++) {

        gpio_gpiod_write(&led, 1);

        printf("Cycle %d: LED ON\n",
               i + 1);

        gpio_sleep_ms(delay_ms);

        gpio_gpiod_write(&led, 0);

        printf("Cycle %d: LED OFF\n",
               i + 1);

        gpio_sleep_ms(delay_ms);
    }

    gpio_gpiod_write(&led, 0);

    gpio_gpiod_close(&led);

    printf("LED test completed.\n");

    return EXIT_SUCCESS;
}
