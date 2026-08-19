#include "gpio_common.h"
#include "gpio_libgpiod.h"

#include <signal.h>


static volatile sig_atomic_t running = 1;


static void signal_handler(int sig)
{
    (void)sig;
    running = 0;
}


static void set_pwm(gpio_gpiod_handle_t *gpio,
                    int duty,
                    int period_us)
{
    int on_time;
    int off_time;

    if (duty < 0)
        duty = 0;

    if (duty > 100)
        duty = 100;

    on_time =
        (period_us * duty) / 100;

    off_time =
        period_us - on_time;

    if (on_time > 0) {
        gpio_gpiod_write(gpio, 1);
        usleep(on_time);
    }

    if (off_time > 0) {
        gpio_gpiod_write(gpio, 0);
        usleep(off_time);
    }
}


int main(int argc, char *argv[])
{
    gpio_gpiod_handle_t gpio;

    int chip = 0;
    int line = 0;

    int period_us = 1000;
    int duty;
    int i;

    if (argc >= 2)
        line = atoi(argv[1]);

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    printf("========================================\n");
    printf(" STM32MP157 PWM Fade Test\n");
    printf("========================================\n");

    printf("GPIO chip : %d\n", chip);
    printf("GPIO line : %d\n", line);

    if (gpio_gpiod_open(&gpio,
                        chip,
                        line) < 0)
        return EXIT_FAILURE;

    if (gpio_gpiod_request_output(
            &gpio,
            "pwm-fade",
            0) < 0) {

        gpio_gpiod_close(&gpio);
        return EXIT_FAILURE;
    }

    printf("PWM fade test running...\n");

    while (running) {

        /*
         * Fade IN.
         */
        for (duty = 0;
             duty <= 100 && running;
             duty++) {

            printf("\rFade IN: %3d%%",
                   duty);

            fflush(stdout);

            /*
             * Run multiple PWM cycles at
             * the selected duty cycle.
             */
            for (i = 0;
                 i < 5 && running;
                 i++) {

                set_pwm(&gpio,
                        duty,
                        period_us);
            }
        }

        /*
         * Fade OUT.
         */
        for (duty = 100;
             duty >= 0 && running;
             duty--) {

            printf("\rFade OUT: %3d%%",
                   duty);

            fflush(stdout);

            for (i = 0;
                 i < 5 && running;
                 i++) {

                set_pwm(&gpio,
                        duty,
                        period_us);
            }
        }
    }

    gpio_gpiod_write(&gpio, 0);

    gpio_gpiod_close(&gpio);

    printf("\nPWM fade stopped.\n");

    return EXIT_SUCCESS;
}
