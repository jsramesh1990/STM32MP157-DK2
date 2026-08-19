/*
 * gpio-common.c
 *
 * Common GPIO utility functions for STM32MP157-DK2
 *
 * Project:
 *     STM32MP157-DK2 Virtual GPIO
 *
 * Purpose:
 *     Provides common validation, logging and GPIO configuration
 *     helpers shared by the GPIO applications.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "gpio_common.h"

/* ------------------------------------------------------------
 * Logging
 * ------------------------------------------------------------ */

void gpio_log(const char *level, const char *message)
{
    if (level == NULL || message == NULL)
        return;

    printf("[GPIO][%s] %s\n", level, message);
}


/* ------------------------------------------------------------
 * GPIO value validation
 * ------------------------------------------------------------ */

int gpio_validate_value(int value)
{
    if (value != GPIO_LOW && value != GPIO_HIGH)
        return -1;

    return 0;
}


/* ------------------------------------------------------------
 * GPIO direction validation
 * ------------------------------------------------------------ */

int gpio_validate_direction(const char *direction)
{
    if (direction == NULL)
        return -1;

    if (strcmp(direction, "in") == 0)
        return 0;

    if (strcmp(direction, "out") == 0)
        return 0;

    return -1;
}


/* ------------------------------------------------------------
 * GPIO configuration initialization
 * ------------------------------------------------------------ */

void gpio_config_init(gpio_config_t *config)
{
    if (config == NULL)
        return;

    memset(config, 0, sizeof(*config));

    config->chip = 0;
    config->line = 0;
    config->direction = GPIO_DIRECTION_INPUT;
    config->value = GPIO_LOW;
    config->active_low = 0;
    config->consumer = "stm32mp157-gpio";
}


/* ------------------------------------------------------------
 * GPIO configuration print
 * ------------------------------------------------------------ */

void gpio_config_print(const gpio_config_t *config)
{
    if (config == NULL)
        return;

    printf("\n");
    printf("========================================\n");
    printf("       GPIO CONFIGURATION\n");
    printf("========================================\n");
    printf("GPIO Chip       : %d\n", config->chip);
    printf("GPIO Line       : %d\n", config->line);
    printf("Direction       : %s\n",
           config->direction == GPIO_DIRECTION_OUTPUT ?
           "OUTPUT" : "INPUT");
    printf("Initial Value   : %d\n", config->value);
    printf("Active Low      : %s\n",
           config->active_low ? "YES" : "NO");
    printf("Consumer        : %s\n",
           config->consumer);
    printf("========================================\n");
    printf("\n");
}


/* ------------------------------------------------------------
 * Error printing
 * ------------------------------------------------------------ */

void gpio_print_error(const char *operation)
{
    if (operation == NULL)
        return;

    fprintf(stderr,
            "[GPIO][ERROR] %s failed: %s\n",
            operation,
            strerror(errno));
}


/* ------------------------------------------------------------
 * GPIO configuration validation
 * ------------------------------------------------------------ */

int gpio_config_validate(const gpio_config_t *config)
{
    if (config == NULL) {
        fprintf(stderr,
                "[GPIO][ERROR] GPIO configuration is NULL\n");
        return -1;
    }

    if (config->chip < 0) {
        fprintf(stderr,
                "[GPIO][ERROR] Invalid GPIO chip: %d\n",
                config->chip);
        return -1;
    }

    if (config->line < 0) {
        fprintf(stderr,
                "[GPIO][ERROR] Invalid GPIO line: %d\n",
                config->line);
        return -1;
    }

    if (config->direction != GPIO_DIRECTION_INPUT &&
        config->direction != GPIO_DIRECTION_OUTPUT) {

        fprintf(stderr,
                "[GPIO][ERROR] Invalid GPIO direction\n");

        return -1;
    }

    if (gpio_validate_value(config->value) < 0) {
        fprintf(stderr,
                "[GPIO][ERROR] Invalid GPIO value: %d\n",
                config->value);

        return -1;
    }

    return 0;
}
