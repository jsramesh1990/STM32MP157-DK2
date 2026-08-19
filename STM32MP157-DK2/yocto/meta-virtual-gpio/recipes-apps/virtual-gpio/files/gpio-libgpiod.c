/*
 * gpio-libgpiod.c
 *
 * libgpiod GPIO implementation for STM32MP157-DK2
 *
 * Uses Linux GPIO character-device interface.
 *
 * Example:
 *
 *     /dev/gpiochip0
 *          |
 *          +-- GPIO line 10
 *          +-- GPIO line 11
 *          +-- GPIO line 12
 *
 * Compile:
 *
 *     gcc gpio-libgpiod.c gpio-common.c \
 *         -lgpiod \
 *         -o gpio-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include <gpiod.h>

#include "gpio_common.h"


/* ------------------------------------------------------------
 * Open GPIO chip
 * ------------------------------------------------------------ */

struct gpiod_chip *gpio_chip_open(int chip_number)
{
    char chip_name[32];

    snprintf(chip_name,
             sizeof(chip_name),
             "gpiochip%d",
             chip_number);

    printf("[GPIO] Opening %s\n", chip_name);

    struct gpiod_chip *chip =
        gpiod_chip_open_by_name(chip_name);

    if (chip == NULL) {
        fprintf(stderr,
                "[GPIO][ERROR] Failed to open %s: %s\n",
                chip_name,
                strerror(errno));

        return NULL;
    }

    printf("[GPIO] GPIO chip opened successfully\n");

    return chip;
}


/* ------------------------------------------------------------
 * Close GPIO chip
 * ------------------------------------------------------------ */

void gpio_chip_close(struct gpiod_chip *chip)
{
    if (chip == NULL)
        return;

    printf("[GPIO] Closing GPIO chip\n");

    gpiod_chip_close(chip);
}


/* ------------------------------------------------------------
 * Request GPIO as output
 * ------------------------------------------------------------ */

struct gpiod_line *gpio_request_output(
    struct gpiod_chip *chip,
    unsigned int line_number,
    int initial_value,
    const char *consumer)
{
    if (chip == NULL) {
        fprintf(stderr,
                "[GPIO][ERROR] Invalid GPIO chip\n");
        return NULL;
    }

    if (gpio_validate_value(initial_value) < 0) {
        fprintf(stderr,
                "[GPIO][ERROR] Invalid initial value\n");
        return NULL;
    }

    if (consumer == NULL)
        consumer = "stm32mp157-gpio";

    struct gpiod_line *line =
        gpiod_chip_get_line(chip, line_number);

    if (line == NULL) {
        fprintf(stderr,
                "[GPIO][ERROR] GPIO line %u not found: %s\n",
                line_number,
                strerror(errno));

        return NULL;
    }

    int ret =
        gpiod_line_request_output(
            line,
            consumer,
            initial_value);

    if (ret < 0) {
        fprintf(stderr,
                "[GPIO][ERROR] Failed to request GPIO line %u "
                "as output: %s\n",
                line_number,
                strerror(errno));

        return NULL;
    }

    printf("[GPIO] GPIO line %u configured as OUTPUT\n",
           line_number);

    printf("[GPIO] Initial value = %d\n",
           initial_value);

    return line;
}


/* ------------------------------------------------------------
 * Request GPIO as input
 * ------------------------------------------------------------ */

struct gpiod_line *gpio_request_input(
    struct gpiod_chip *chip,
    unsigned int line_number,
    const char *consumer)
{
    if (chip == NULL) {
        fprintf(stderr,
                "[GPIO][ERROR] Invalid GPIO chip\n");
        return NULL;
    }

    if (consumer == NULL)
        consumer = "stm32mp157-gpio";

    struct gpiod_line *line =
        gpiod_chip_get_line(chip, line_number);

    if (line == NULL) {
        fprintf(stderr,
                "[GPIO][ERROR] GPIO line %u not found: %s\n",
                line_number,
                strerror(errno));

        return NULL;
    }

    int ret =
        gpiod_line_request_input(
            line,
            consumer);

    if (ret < 0) {
        fprintf(stderr,
                "[GPIO][ERROR] Failed to request GPIO line %u "
                "as input: %s\n",
                line_number,
                strerror(errno));

        return NULL;
    }

    printf("[GPIO] GPIO line %u configured as INPUT\n",
           line_number);

    return line;
}


/* ------------------------------------------------------------
 * Set GPIO value
 * ------------------------------------------------------------ */

int gpio_set_value(
    struct gpiod_line *line,
    int value)
{
    if (line == NULL) {
        fprintf(stderr,
                "[GPIO][ERROR] Invalid GPIO line\n");
        return -1;
    }

    if (gpio_validate_value(value) < 0) {
        fprintf(stderr,
                "[GPIO][ERROR] Invalid GPIO value: %d\n",
                value);
        return -1;
    }

    int ret =
        gpiod_line_set_value(line, value);

    if (ret < 0) {
        fprintf(stderr,
                "[GPIO][ERROR] Failed to set GPIO value: %s\n",
                strerror(errno));

        return -1;
    }

    printf("[GPIO] Value changed -> %d\n",
           value);

    return 0;
}


/* ------------------------------------------------------------
 * Get GPIO value
 * ------------------------------------------------------------ */

int gpio_get_value(
    struct gpiod_line *line)
{
    if (line == NULL) {
        fprintf(stderr,
                "[GPIO][ERROR] Invalid GPIO line\n");
        return -1;
    }

    int value =
        gpiod_line_get_value(line);

    if (value < 0) {
        fprintf(stderr,
                "[GPIO][ERROR] Failed to read GPIO: %s\n",
                strerror(errno));

        return -1;
    }

    return value;
}


/* ------------------------------------------------------------
 * Release GPIO line
 * ------------------------------------------------------------ */

void gpio_line_release(
    struct gpiod_line *line)
{
    if (line == NULL)
        return;

    printf("[GPIO] Releasing GPIO line\n");

    gpiod_line_release(line);
}


/* ------------------------------------------------------------
 * GPIO toggle
 * ------------------------------------------------------------ */

int gpio_toggle(
    struct gpiod_line *line)
{
    int current_value;

    current_value = gpio_get_value(line);

    if (current_value < 0)
        return -1;

    int new_value =
        current_value ? GPIO_LOW : GPIO_HIGH;

    return gpio_set_value(line, new_value);
}


/* ------------------------------------------------------------
 * GPIO blink
 * ------------------------------------------------------------ */

int gpio_blink(
    struct gpiod_line *line,
    int count,
    unsigned int delay_ms)
{
    if (line == NULL)
        return -1;

    if (count <= 0)
        return -1;

    printf("[GPIO] Starting blink sequence\n");
    printf("[GPIO] Count = %d\n", count);
    printf("[GPIO] Delay = %u ms\n", delay_ms);

    for (int i = 0; i < count; i++) {

        if (gpio_set_value(line, GPIO_HIGH) < 0)
            return -1;

        usleep(delay_ms * 1000);

        if (gpio_set_value(line, GPIO_LOW) < 0)
            return -1;

        usleep(delay_ms * 1000);

        printf("[GPIO] Blink %d/%d\n",
               i + 1,
               count);
    }

    return 0;
}
