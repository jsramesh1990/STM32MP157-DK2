#include "gpio_common.h"
#include "gpio_libgpiod.h"

#include <signal.h>


static volatile sig_atomic_t running = 1;


static void signal_handler(int sig)
{
    (void)sig;
    running = 0;
}


/*
 * Software PWM demonstration.
 *
 * IMPORTANT:
 * This is educational software PWM.
 * For production hardware PWM on STM32MP157,
 * use the Linux PWM subsystem:
 *
 *     /sys/class/pwm/
 *
 * or the corresponding libgpiod/device-tree
 * hardware configuration where applicable.
 */
static void pwm_cycle(gpio_gpiod_handle_t *gpio,
                      int duty,
                      int period_ms)
{
    int on_time;
    int off_time;

    if (duty < 0)
        duty = 0;

    if (duty > 100)
        duty = 100;

    on_time =
        (period_ms * duty) / 100;

    off_time =
        period_ms - on_time;

    if (on_time > 0) {
        gpio_gpiod_write(gpio, 1);
        usleep(on_time * 1000);
    }

    if (off_time > 0) {
        gpio_gpiod_write(gpio, 0);
        usleep(off_time * 1000);
    }
}


int main(int argc, char *argv[])
{
    gpio_gpiod_handle_t led;

    int chip = 0;
    int line = 0;

    int period_ms = 20;
    int duty;

    if (argc >= 2)
        line = atoi(argv[1]);

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    printf("========================================\n");
    printf(" STM32MP157 Software PWM LED\n");
    printf("========================================\n");

    printf("GPIO chip : %d\n", chip);
    printf("GPIO line : %d\n", line);

    if (gpio_gpiod_open(&led,
                        chip,
                        line) < 0)
        return EXIT_FAILURE;

    if (gpio_gpiod_request_output(
            &led,
            "pwm-led",
            0) < 0) {

        gpio_gpiod_close(&led);
        return EXIT_FAILURE;
    }

    printf("PWM fading started.\n");

    while (running) {

        for (duty = 0;
             duty <= 100 && running;
             duty++) {

            printf("\rDuty Cycle: %3d%%",
                   duty);

            fflush(stdout);

            pwm_cycle(&led,
                      duty,
                      period_ms);
        }

        for (duty = 100;
             duty >= 0 && running;
             duty--) {

            printf("\rDuty Cycle: %3d%%",
                   duty);

            fflush(stdout);

            pwm_cycle(&led,
                      duty,
                      period_ms);
        }
    }

    gpio_gpiod_write(&led, 0);

    gpio_gpiod_close(&led);

    printf("\nPWM stopped.\n");

    return EXIT_SUCCESS;
}
