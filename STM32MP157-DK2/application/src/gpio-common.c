/*
 * gpio-common.c
 *
 * Common GPIO abstraction layer
 *
 * Target:
 *     STM32MP157-DK2
 *
 * Provides a unified interface for:
 *
 *     1. libgpiod
 *     2. GPIO Sysfs
 *
 * Recommended interface:
 *
 *     libgpiod
 *
 * Legacy compatibility:
 *
 *     GPIO Sysfs
 */

#include "gpio_common.h"
#include "gpio_libgpiod.h"
#include "gpio_sysfs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


/* ---------------------------------------------------------
 * Internal helper
 * --------------------------------------------------------- */

static int gpio_validate_config(const gpio_config_t *config)
{
    if (!config) {
        fprintf(stderr,
                "GPIO error: NULL configuration\n");
        return GPIO_ERROR;
    }

    if (config->line < 0) {
        fprintf(stderr,
                "GPIO error: invalid line number\n");
        return GPIO_ERROR;
    }

    if (config->chip < 0) {
        fprintf(stderr,
                "GPIO error: invalid GPIO chip\n");
        return GPIO_ERROR;
    }

    return GPIO_SUCCESS;
}


/* ---------------------------------------------------------
 * Initialize GPIO
 *
 * Automatically selects:
 *
 *     libgpiod
 *         OR
 *     sysfs
 *
 * depending on configuration.
 * --------------------------------------------------------- */

int gpio_init(gpio_config_t *config)
{
    if (gpio_validate_config(config) < 0)
        return GPIO_ERROR;

    /*
     * Already initialized.
     */
    if (config->initialized)
        return GPIO_SUCCESS;


    /* -----------------------------------------------------
     * libgpiod mode
     * ----------------------------------------------------- */

    if (config->use_libgpiod) {

        if (gpio_gpiod_init(&config->gpiod,
                            config->chip,
                            config->line) < 0) {

            fprintf(stderr,
                    "libgpiod initialization failed\n");

            return GPIO_ERROR;
        }

        /*
         * Configure initial direction.
         */

        if (strcmp(config->direction, "out") == 0) {

            if (gpio_gpiod_config_output(
                    &config->gpiod,
                    config->value) < 0) {

                gpio_gpiod_release(&config->gpiod);

                return GPIO_ERROR;
            }

        } else if (strcmp(config->direction, "in") == 0) {

            if (gpio_gpiod_config_input(
                    &config->gpiod) < 0) {

                gpio_gpiod_release(&config->gpiod);

                return GPIO_ERROR;
            }

        } else {

            fprintf(stderr,
                    "Invalid GPIO direction: %s\n",
                    config->direction);

            gpio_gpiod_release(&config->gpiod);

            return GPIO_ERROR;
        }

        config->initialized = 1;
        config->backend = GPIO_BACKEND_LIBGPIOD;

        return GPIO_SUCCESS;
    }


    /* -----------------------------------------------------
     * Sysfs mode
     * ----------------------------------------------------- */

    if (gpio_export(config->line) < 0) {

        fprintf(stderr,
                "Failed to export GPIO%d\n",
                config->line);

        return GPIO_ERROR;
    }


    if (strcmp(config->direction, "out") == 0) {

        if (gpio_config_output(config->line,
                               config->value) < 0) {

            gpio_unexport(config->line);

            return GPIO_ERROR;
        }

    } else if (strcmp(config->direction, "in") == 0) {

        if (gpio_config_input(config->line) < 0) {

            gpio_unexport(config->line);

            return GPIO_ERROR;
        }

    } else {

        fprintf(stderr,
                "Invalid GPIO direction: %s\n",
                config->direction);

        gpio_unexport(config->line);

        return GPIO_ERROR;
    }

    config->initialized = 1;
    config->backend = GPIO_BACKEND_SYSFS;

    return GPIO_SUCCESS;
}


/* ---------------------------------------------------------
 * GPIO write
 * --------------------------------------------------------- */

int gpio_write(gpio_config_t *config,
               int value)
{
    if (!config || !config->initialized) {

        fprintf(stderr,
                "GPIO is not initialized\n");

        return GPIO_ERROR;
    }

    if (value != GPIO_LOW &&
        value != GPIO_HIGH) {

        fprintf(stderr,
                "Invalid GPIO value: %d\n",
                value);

        return GPIO_ERROR;
    }


    /* -----------------------------------------------------
     * libgpiod backend
     * ----------------------------------------------------- */

    if (config->backend ==
        GPIO_BACKEND_LIBGPIOD) {

        if (gpio_gpiod_write(
                &config->gpiod,
                value) < 0)
            return GPIO_ERROR;

    }


    /* -----------------------------------------------------
     * Sysfs backend
     * ----------------------------------------------------- */

    else if (config->backend ==
             GPIO_BACKEND_SYSFS) {

        if (gpio_set_value(
                config->line,
                value) < 0)
            return GPIO_ERROR;
    }

    else {

        fprintf(stderr,
                "Unknown GPIO backend\n");

        return GPIO_ERROR;
    }


    /*
     * Update cached value.
     */

    config->value = value;

    return GPIO_SUCCESS;
}


/* ---------------------------------------------------------
 * GPIO read
 * --------------------------------------------------------- */

int gpio_read(gpio_config_t *config)
{
    int value;

    if (!config || !config->initialized) {

        fprintf(stderr,
                "GPIO is not initialized\n");

        return GPIO_ERROR;
    }


    /* -----------------------------------------------------
     * libgpiod backend
     * ----------------------------------------------------- */

    if (config->backend ==
        GPIO_BACKEND_LIBGPIOD) {

        value = gpio_gpiod_read(
                    &config->gpiod);

        if (value < 0)
            return GPIO_ERROR;
    }


    /* -----------------------------------------------------
     * Sysfs backend
     * ----------------------------------------------------- */

    else if (config->backend ==
             GPIO_BACKEND_SYSFS) {

        value = gpio_get_value(
                    config->line);

        if (value < 0)
            return GPIO_ERROR;
    }

    else {

        fprintf(stderr,
                "Unknown GPIO backend\n");

        return GPIO_ERROR;
    }


    /*
     * Update cached value.
     */

    config->value = value;

    return value;
}


/* ---------------------------------------------------------
 * GPIO toggle
 * --------------------------------------------------------- */

int gpio_toggle(gpio_config_t *config)
{
    int current;
    int new_value;

    if (!config || !config->initialized) {

        fprintf(stderr,
                "GPIO is not initialized\n");

        return GPIO_ERROR;
    }


    /*
     * Read current GPIO state.
     */

    current = gpio_read(config);

    if (current < 0)
        return GPIO_ERROR;


    /*
     * Toggle.
     */

    new_value = !current;


    /*
     * Write new state.
     */

    if (gpio_write(config,
                   new_value) < 0)
        return GPIO_ERROR;

    return new_value;
}


/* ---------------------------------------------------------
 * GPIO cleanup
 * --------------------------------------------------------- */

int gpio_cleanup(gpio_config_t *config)
{
    if (!config)
        return GPIO_ERROR;

    if (!config->initialized)
        return GPIO_SUCCESS;


    /* -----------------------------------------------------
     * libgpiod backend
     * ----------------------------------------------------- */

    if (config->backend ==
        GPIO_BACKEND_LIBGPIOD) {

        gpio_gpiod_release(
            &config->gpiod);
    }


    /* -----------------------------------------------------
     * Sysfs backend
     * ----------------------------------------------------- */

    else if (config->backend ==
             GPIO_BACKEND_SYSFS) {

        gpio_unexport(
            config->line);
    }


    /*
     * Reset configuration state.
     */

    config->initialized = 0;
    config->backend = GPIO_BACKEND_NONE;
    config->value = 0;

    return GPIO_SUCCESS;
}


/* ---------------------------------------------------------
 * Set GPIO direction
 * --------------------------------------------------------- */

int gpio_set_direction(gpio_config_t *config,
                       const char *direction)
{
    if (!config || !config->initialized)
        return GPIO_ERROR;

    if (!direction)
        return GPIO_ERROR;


    /* -----------------------------------------------------
     * libgpiod
     * ----------------------------------------------------- */

    if (config->backend ==
        GPIO_BACKEND_LIBGPIOD) {

        if (strcmp(direction, "out") == 0) {

            if (gpio_gpiod_config_output(
                    &config->gpiod,
                    config->value) < 0)
                return GPIO_ERROR;

        } else if (strcmp(direction, "in") == 0) {

            if (gpio_gpiod_config_input(
                    &config->gpiod) < 0)
                return GPIO_ERROR;

        } else {

            fprintf(stderr,
                    "Unsupported GPIO direction: %s\n",
                    direction);

            return GPIO_ERROR;
        }
    }


    /* -----------------------------------------------------
     * Sysfs
     * ----------------------------------------------------- */

    else if (config->backend ==
             GPIO_BACKEND_SYSFS) {

        if (gpio_set_direction(
                config->line,
                direction) < 0)
            return GPIO_ERROR;
    }


    else {

        fprintf(stderr,
                "Unknown GPIO backend\n");

        return GPIO_ERROR;
    }


    /*
     * Save direction.
     */

    strncpy(config->direction,
            direction,
            sizeof(config->direction) - 1);

    config->direction[
        sizeof(config->direction) - 1] = '\0';

    return GPIO_SUCCESS;
}


/* ---------------------------------------------------------
 * GPIO status
 * --------------------------------------------------------- */

void gpio_print_status(const gpio_config_t *config)
{
    if (!config) {
        printf("GPIO: NULL configuration\n");
        return;
    }

    printf("\n");
    printf("========================================\n");
    printf("          GPIO STATUS\n");
    printf("========================================\n");

    printf("Chip          : %d\n",
           config->chip);

    printf("Line          : %d\n",
           config->line);

    printf("Direction     : %s\n",
           config->direction);

    printf("Value         : %d\n",
           config->value);

    printf("Initialized    : %s\n",
           config->initialized ?
           "YES" : "NO");

    printf("Backend       : ");

    switch (config->backend) {

    case GPIO_BACKEND_LIBGPIOD:
        printf("libgpiod\n");
        break;

    case GPIO_BACKEND_SYSFS:
        printf("sysfs\n");
        break;

    default:
        printf("none\n");
        break;
    }

    printf("========================================\n");
    printf("\n");
}


/* ---------------------------------------------------------
 * GPIO backend name
 * --------------------------------------------------------- */

const char *gpio_backend_name(
        const gpio_config_t *config)
{
    if (!config)
        return "none";

    switch (config->backend) {

    case GPIO_BACKEND_LIBGPIOD:
        return "libgpiod";

    case GPIO_BACKEND_SYSFS:
        return "sysfs";

    default:
        return "none";
    }
}
