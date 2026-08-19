/*
 * pwm_fade.c
 *
 * STM32MP157-DK2
 * PWM LED Fade Example
 *
 * Description:
 *   Generates a hardware PWM signal and gradually
 *   changes the duty cycle.
 *
 * Usage:
 *
 *   sudo ./pwm_fade <chip> <channel>
 *
 * Example:
 *
 *   sudo ./pwm_fade 0 0
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <errno.h>

#define PWM_BASE_PATH "/sys/class/pwm"

#define DEFAULT_PERIOD_NS 1000000UL
#define STEP_NS           10000UL
#define STEP_DELAY_US     10000

static volatile sig_atomic_t running = 1;

static void signal_handler(int signal)
{
    (void)signal;

    running = 0;
}

static int write_value(const char *path,
                       const char *value)
{
    FILE *fp;

    fp = fopen(path, "w");

    if (!fp) {
        fprintf(stderr,
                "ERROR: %s: %s\n",
                path,
                strerror(errno));

        return -1;
    }

    if (fprintf(fp, "%s", value) < 0) {

        fprintf(stderr,
                "ERROR: Write failed: %s\n",
                path);

        fclose(fp);

        return -1;
    }

    fclose(fp);

    return 0;
}

static int pwm_export(int chip,
                      int channel)
{
    char path[256];
    char value[32];

    snprintf(path,
             sizeof(path),
             PWM_BASE_PATH
             "/pwmchip%d/export",
             chip);

    snprintf(value,
             sizeof(value),
             "%d",
             channel);

    return write_value(path, value);
}

static int pwm_unexport(int chip,
                        int channel)
{
    char path[256];
    char value[32];

    snprintf(path,
             sizeof(path),
             PWM_BASE_PATH
             "/pwmchip%d/unexport",
             chip);

    snprintf(value,
             sizeof(value),
             "%d",
             channel);

    return write_value(path, value);
}

static int pwm_set_period(int chip,
                          int channel,
                          unsigned long period)
{
    char path[256];
    char value[64];

    snprintf(path,
             sizeof(path),
             PWM_BASE_PATH
             "/pwmchip%d/pwm%d/period",
             chip,
             channel);

    snprintf(value,
             sizeof(value),
             "%lu",
             period);

    return write_value(path, value);
}

static int pwm_set_duty(int chip,
                        int channel,
                        unsigned long duty)
{
    char path[256];
    char value[64];

    snprintf(path,
             sizeof(path),
             PWM_BASE_PATH
             "/pwmchip%d/pwm%d/duty_cycle",
             chip,
             channel);

    snprintf(value,
             sizeof(value),
             "%lu",
             duty);

    return write_value(path, value);
}

static int pwm_enable(int chip,
                      int channel)
{
    char path[256];

    snprintf(path,
             sizeof(path),
             PWM_BASE_PATH
             "/pwmchip%d/pwm%d/enable",
             chip,
             channel);

    return write_value(path, "1");
}

static int pwm_disable(int chip,
                       int channel)
{
    char path[256];

    snprintf(path,
             sizeof(path),
             PWM_BASE_PATH
             "/pwmchip%d/pwm%d/enable",
             chip,
             channel);

    return write_value(path, "0");
}

static void show_usage(const char *program)
{
    printf("\n");
    printf("STM32MP157-DK2 PWM Fade Example\n");
    printf("--------------------------------\n\n");

    printf("Usage:\n");

    printf("  sudo %s <chip> <channel>\n\n",
           program);

    printf("Example:\n");

    printf("  sudo %s 0 0\n\n",
           program);
}

static int configure_pwm(int chip,
                         int channel)
{
    char path[256];

    /*
     * Check whether channel is already exported.
     */
    snprintf(path,
             sizeof(path),
             PWM_BASE_PATH
             "/pwmchip%d/pwm%d",
             chip,
             channel);

    if (access(path, F_OK) != 0) {

        printf("Exporting PWM channel...\n");

        if (pwm_export(chip, channel) < 0) {
            return -1;
        }

        usleep(100000);
    }

    /*
     * Set PWM period.
     */
    if (pwm_set_period(chip,
                       channel,
                       DEFAULT_PERIOD_NS) < 0) {

        return -1;
    }

    /*
     * Start with 0% duty.
     */
    if (pwm_set_duty(chip,
                     channel,
                     0) < 0) {

        return -1;
    }

    /*
     * Enable PWM.
     */
    if (pwm_enable(chip,
                   channel) < 0) {

        return -1;
    }

    return 0;
}

static void fade_in(int chip,
                    int channel)
{
    unsigned long duty;

    printf("\nFade IN\n");

    for (duty = 0;
         duty <= DEFAULT_PERIOD_NS && running;
         duty += STEP_NS) {

        pwm_set_duty(chip,
                     channel,
                     duty);

        printf("\rBrightness: %3lu%%",
               (duty * 100) /
               DEFAULT_PERIOD_NS);

        fflush(stdout);

        usleep(STEP_DELAY_US);
    }

    printf("\n");
}

static void fade_out(int chip,
                     int channel)
{
    long duty;

    printf("\nFade OUT\n");

    for (duty = DEFAULT_PERIOD_NS;
         duty >= 0 && running;
         duty -= STEP_NS) {

        pwm_set_duty(chip,
                     channel,
                     (unsigned long)duty);

        printf("\rBrightness: %3ld%%",
               (duty * 100) /
               (long)DEFAULT_PERIOD_NS);

        fflush(stdout);

        usleep(STEP_DELAY_US);
    }

    printf("\n");
}

int main(int argc,
         char *argv[])
{
    int chip;
    int channel;

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    if (argc != 3) {

        show_usage(argv[0]);

        return EXIT_FAILURE;
    }

    chip = atoi(argv[1]);
    channel = atoi(argv[2]);

    printf("========================================\n");
    printf(" STM32MP157-DK2 PWM Fade Test\n");
    printf("========================================\n");

    printf("PWM Chip   : %d\n", chip);
    printf("PWM Channel: %d\n", channel);
    printf("Period     : %lu ns\n",
           DEFAULT_PERIOD_NS);

    /*
     * Configure PWM.
     */
    if (configure_pwm(chip,
                      channel) < 0) {

        fprintf(stderr,
                "ERROR: PWM configuration failed\n");

        return EXIT_FAILURE;
    }

    printf("PWM configured successfully.\n");

    printf("\nStarting LED fade.\n");
    printf("Press Ctrl+C to stop.\n");

    /*
     * Continuous fade.
     */
    while (running) {

        fade_in(chip, channel);

        if (!running)
            break;

        sleep(1);

        fade_out(chip, channel);

        if (!running)
            break;

        sleep(1);
    }

    printf("\nStopping PWM...\n");

    /*
     * Set duty cycle to 0 before disabling.
     */
    pwm_set_duty(chip,
                 channel,
                 0);

    pwm_disable(chip,
                channel);

    pwm_unexport(chip,
                 channel);

    printf("PWM stopped successfully.\n");

    return EXIT_SUCCESS;
}
