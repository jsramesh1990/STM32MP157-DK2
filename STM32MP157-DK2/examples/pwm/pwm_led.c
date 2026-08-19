/*
 * pwm_led.c
 *
 * STM32MP157-DK2
 * Linux PWM LED Control Example
 *
 * Description:
 *   Controls LED brightness using hardware PWM.
 *
 * Usage:
 *   sudo ./pwm_led <chip> <channel> <period_ns> <duty_ns>
 *
 * Example:
 *   sudo ./pwm_led 0 0 1000000 500000
 *
 * period = 1 ms
 * duty   = 0.5 ms
 * duty cycle = 50%
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#define PWM_BASE_PATH "/sys/class/pwm"

static int write_value(const char *path, const char *value)
{
    FILE *fp;

    fp = fopen(path, "w");

    if (!fp) {
        fprintf(stderr,
                "ERROR: Cannot open %s: %s\n",
                path,
                strerror(errno));
        return -1;
    }

    if (fprintf(fp, "%s", value) < 0) {
        fprintf(stderr,
                "ERROR: Cannot write %s\n",
                path);
        fclose(fp);
        return -1;
    }

    fclose(fp);

    return 0;
}

static int read_value(const char *path, char *buffer, size_t size)
{
    FILE *fp;

    fp = fopen(path, "r");

    if (!fp) {
        fprintf(stderr,
                "ERROR: Cannot open %s: %s\n",
                path,
                strerror(errno));
        return -1;
    }

    if (!fgets(buffer, size, fp)) {
        fclose(fp);
        return -1;
    }

    fclose(fp);

    return 0;
}

static int pwm_export(int chip, int channel)
{
    char path[256];
    char channel_str[32];

    snprintf(path,
             sizeof(path),
             PWM_BASE_PATH "/pwmchip%d/export",
             chip);

    snprintf(channel_str,
             sizeof(channel_str),
             "%d",
             channel);

    return write_value(path, channel_str);
}

static int pwm_unexport(int chip, int channel)
{
    char path[256];
    char channel_str[32];

    snprintf(path,
             sizeof(path),
             PWM_BASE_PATH "/pwmchip%d/unexport",
             chip);

    snprintf(channel_str,
             sizeof(channel_str),
             "%d",
             channel);

    return write_value(path, channel_str);
}

static int pwm_configure(int chip,
                         int channel,
                         unsigned long period,
                         unsigned long duty)
{
    char path[256];
    char value[64];

    /*
     * Set period.
     */
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

    if (write_value(path, value) < 0)
        return -1;

    /*
     * Set duty cycle.
     */
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

    if (write_value(path, value) < 0)
        return -1;

    return 0;
}

static int pwm_enable(int chip, int channel)
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

static int pwm_disable(int chip, int channel)
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

static void print_usage(const char *program)
{
    printf("\n");
    printf("STM32MP157-DK2 PWM LED Example\n");
    printf("--------------------------------\n\n");

    printf("Usage:\n");

    printf("  sudo %s <chip> <channel> "
           "<period_ns> <duty_ns>\n\n",
           program);

    printf("Example:\n");

    printf("  sudo %s 0 0 1000000 500000\n\n",
           program);

    printf("Parameters:\n");
    printf("  chip       PWM chip number\n");
    printf("  channel    PWM channel number\n");
    printf("  period_ns  PWM period in nanoseconds\n");
    printf("  duty_ns    Duty cycle in nanoseconds\n\n");

    printf("Example:\n");
    printf("  Period = 1000000 ns = 1 ms\n");
    printf("  Duty   = 500000 ns  = 0.5 ms\n");
    printf("  Duty   = 50%%\n\n");
}

int main(int argc, char *argv[])
{
    int chip;
    int channel;

    unsigned long period;
    unsigned long duty;

    char path[256];
    char buffer[128];

    if (argc != 5) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    chip = atoi(argv[1]);
    channel = atoi(argv[2]);

    period = strtoul(argv[3], NULL, 10);
    duty = strtoul(argv[4], NULL, 10);

    if (period == 0) {
        fprintf(stderr,
                "ERROR: Period cannot be zero\n");
        return EXIT_FAILURE;
    }

    if (duty > period) {
        fprintf(stderr,
                "ERROR: Duty cycle cannot exceed period\n");
        return EXIT_FAILURE;
    }

    printf("========================================\n");
    printf(" STM32MP157-DK2 PWM LED Test\n");
    printf("========================================\n");

    printf("PWM Chip   : %d\n", chip);
    printf("PWM Channel: %d\n", channel);
    printf("Period     : %lu ns\n", period);
    printf("Duty       : %lu ns\n", duty);

    printf("Duty Cycle : %.2f%%\n",
           ((double)duty / (double)period) * 100.0);

    /*
     * Check if PWM channel already exists.
     */
    snprintf(path,
             sizeof(path),
             PWM_BASE_PATH
             "/pwmchip%d/pwm%d",
             chip,
             channel);

    if (access(path, F_OK) != 0) {

        printf("\nExporting PWM channel...\n");

        if (pwm_export(chip, channel) < 0) {
            return EXIT_FAILURE;
        }

        /*
         * Give kernel time to create the channel.
         */
        usleep(100000);
    }

    /*
     * Configure PWM.
     */
    printf("Configuring PWM...\n");

    if (pwm_configure(chip,
                      channel,
                      period,
                      duty) < 0) {

        fprintf(stderr,
                "ERROR: PWM configuration failed\n");

        return EXIT_FAILURE;
    }

    /*
     * Enable PWM.
     */
    printf("Enabling PWM...\n");

    if (pwm_enable(chip, channel) < 0) {
        fprintf(stderr,
                "ERROR: Failed to enable PWM\n");

        return EXIT_FAILURE;
    }

    printf("\nPWM running...\n");
    printf("Press Ctrl+C to stop.\n");

    /*
     * Keep PWM active.
     */
    while (1) {

        sleep(1);

        /*
         * Read current PWM state.
         */
        snprintf(path,
                 sizeof(path),
                 PWM_BASE_PATH
                 "/pwmchip%d/pwm%d/duty_cycle",
                 chip,
                 channel);

        if (read_value(path,
                       buffer,
                       sizeof(buffer)) == 0) {

            printf("\rCurrent duty: %s",
                   buffer);

            fflush(stdout);
        }
    }

    /*
     * Normally reached only if loop is changed.
     */
    pwm_disable(chip, channel);

    pwm_unexport(chip, channel);

    return EXIT_SUCCESS;
}
