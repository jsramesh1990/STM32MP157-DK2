/*
 * gpio-sysfs.c
 *
 * Linux GPIO Sysfs implementation for STM32MP157-DK2
 *
 * This module provides GPIO control through the legacy:
 *
 *     /sys/class/gpio/
 *
 * interface.
 *
 * NOTE:
 * GPIO Sysfs is deprecated in modern Linux kernels.
 * Prefer libgpiod for new applications.
 */

#include "gpio_sysfs.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>


/* ---------------------------------------------------------
 * Internal helper
 * --------------------------------------------------------- */

static int gpio_validate(unsigned int gpio)
{
    if (gpio >= GPIO_MAX_NUMBER) {
        fprintf(stderr,
                "GPIO error: invalid GPIO number %u\n",
                gpio);
        return GPIO_ERROR;
    }

    return GPIO_SUCCESS;
}


/* ---------------------------------------------------------
 * Build sysfs GPIO path
 * --------------------------------------------------------- */

static int gpio_build_path(unsigned int gpio,
                           const char *attribute,
                           char *path,
                           size_t size)
{
    if (!path || size == 0)
        return GPIO_ERROR;

    if (gpio_validate(gpio) < 0)
        return GPIO_ERROR;

    if (attribute == NULL) {
        snprintf(path,
                 size,
                 "%s/gpio%u",
                 GPIO_SYSFS_PATH,
                 gpio);
    } else {
        snprintf(path,
                 size,
                 "%s/gpio%u/%s",
                 GPIO_SYSFS_PATH,
                 gpio,
                 attribute);
    }

    return GPIO_SUCCESS;
}


/* ---------------------------------------------------------
 * Check whether GPIO is exported
 * --------------------------------------------------------- */

int gpio_is_exported(unsigned int gpio)
{
    char path[128];

    if (gpio_build_path(gpio, NULL, path, sizeof(path)) < 0)
        return GPIO_ERROR;

    if (access(path, F_OK) == 0)
        return 1;

    if (errno == ENOENT)
        return 0;

    return GPIO_ERROR;
}


/* ---------------------------------------------------------
 * Write GPIO sysfs attribute
 * --------------------------------------------------------- */

int gpio_sysfs_write(unsigned int gpio,
                     const char *attribute,
                     const char *value)
{
    char path[128];
    int fd;
    ssize_t ret;

    if (!attribute || !value)
        return GPIO_ERROR;

    if (gpio_build_path(gpio,
                        attribute,
                        path,
                        sizeof(path)) < 0)
        return GPIO_ERROR;

    fd = open(path, O_WRONLY);

    if (fd < 0) {
        perror("open GPIO sysfs attribute");
        return GPIO_ERROR;
    }

    ret = write(fd, value, strlen(value));

    if (ret < 0) {
        perror("write GPIO sysfs attribute");
        close(fd);
        return GPIO_ERROR;
    }

    close(fd);

    return GPIO_SUCCESS;
}


/* ---------------------------------------------------------
 * Read GPIO sysfs attribute
 * --------------------------------------------------------- */

int gpio_sysfs_read(unsigned int gpio,
                    const char *attribute,
                    char *buffer,
                    size_t size)
{
    char path[128];
    int fd;
    ssize_t ret;

    if (!attribute || !buffer || size == 0)
        return GPIO_ERROR;

    if (gpio_build_path(gpio,
                        attribute,
                        path,
                        sizeof(path)) < 0)
        return GPIO_ERROR;

    fd = open(path, O_RDONLY);

    if (fd < 0) {
        perror("open GPIO sysfs attribute");
        return GPIO_ERROR;
    }

    ret = read(fd, buffer, size - 1);

    if (ret < 0) {
        perror("read GPIO sysfs attribute");
        close(fd);
        return GPIO_ERROR;
    }

    buffer[ret] = '\0';

    close(fd);

    return GPIO_SUCCESS;
}


/* ---------------------------------------------------------
 * Export GPIO
 *
 * Equivalent:
 *
 * echo <gpio> > /sys/class/gpio/export
 * --------------------------------------------------------- */

int gpio_export(unsigned int gpio)
{
    char gpio_number[32];
    int fd;
    ssize_t ret;

    if (gpio_validate(gpio) < 0)
        return GPIO_ERROR;

    /*
     * If GPIO is already exported,
     * there is nothing to do.
     */
    if (gpio_is_exported(gpio) == 1)
        return GPIO_SUCCESS;

    fd = open(GPIO_SYSFS_PATH "/export", O_WRONLY);

    if (fd < 0) {
        perror("open GPIO export");
        return GPIO_ERROR;
    }

    snprintf(gpio_number,
             sizeof(gpio_number),
             "%u",
             gpio);

    ret = write(fd,
                gpio_number,
                strlen(gpio_number));

    if (ret < 0) {
        perror("export GPIO");
        close(fd);
        return GPIO_ERROR;
    }

    close(fd);

    /*
     * Wait for kernel to create gpioX directory.
     */
    if (gpio_wait_ready(gpio, 500) < 0) {
        fprintf(stderr,
                "GPIO%u export timeout\n",
                gpio);
        return GPIO_ERROR;
    }

    return GPIO_SUCCESS;
}


/* ---------------------------------------------------------
 * Unexport GPIO
 *
 * Equivalent:
 *
 * echo <gpio> > /sys/class/gpio/unexport
 * --------------------------------------------------------- */

int gpio_unexport(unsigned int gpio)
{
    char gpio_number[32];
    int fd;
    ssize_t ret;

    if (gpio_validate(gpio) < 0)
        return GPIO_ERROR;

    if (gpio_is_exported(gpio) != 1)
        return GPIO_SUCCESS;

    fd = open(GPIO_SYSFS_PATH "/unexport", O_WRONLY);

    if (fd < 0) {
        perror("open GPIO unexport");
        return GPIO_ERROR;
    }

    snprintf(gpio_number,
             sizeof(gpio_number),
             "%u",
             gpio);

    ret = write(fd,
                gpio_number,
                strlen(gpio_number));

    if (ret < 0) {
        perror("unexport GPIO");
        close(fd);
        return GPIO_ERROR;
    }

    close(fd);

    return GPIO_SUCCESS;
}


/* ---------------------------------------------------------
 * Wait until GPIO sysfs directory is available
 * --------------------------------------------------------- */

int gpio_wait_ready(unsigned int gpio,
                    unsigned int timeout_ms)
{
    unsigned int elapsed = 0;

    while (elapsed < timeout_ms) {

        if (gpio_is_exported(gpio) == 1)
            return GPIO_SUCCESS;

        usleep(1000);

        elapsed++;
    }

    return GPIO_ERROR;
}


/* ---------------------------------------------------------
 * Set GPIO direction
 *
 * Supported:
 *
 *   in
 *   out
 *   high
 *   low
 * --------------------------------------------------------- */

int gpio_set_direction(unsigned int gpio,
                       const char *direction)
{
    if (!direction)
        return GPIO_ERROR;

    if (strcmp(direction, GPIO_DIRECTION_IN) != 0 &&
        strcmp(direction, GPIO_DIRECTION_OUT) != 0 &&
        strcmp(direction, GPIO_DIRECTION_HIGH) != 0 &&
        strcmp(direction, GPIO_DIRECTION_LOW) != 0) {

        fprintf(stderr,
                "Invalid GPIO direction: %s\n",
                direction);

        return GPIO_ERROR;
    }

    return gpio_sysfs_write(gpio,
                            "direction",
                            direction);
}


/* ---------------------------------------------------------
 * Configure GPIO as output
 * --------------------------------------------------------- */

int gpio_config_output(unsigned int gpio,
                       unsigned int value)
{
    if (value > 1) {
        fprintf(stderr,
                "Invalid GPIO value: %u\n",
                value);

        return GPIO_ERROR;
    }

    if (gpio_export(gpio) < 0)
        return GPIO_ERROR;

    if (gpio_set_direction(gpio,
                           GPIO_DIRECTION_OUT) < 0)
        return GPIO_ERROR;

    if (gpio_set_value(gpio, value) < 0)
        return GPIO_ERROR;

    return GPIO_SUCCESS;
}


/* ---------------------------------------------------------
 * Configure GPIO as input
 * --------------------------------------------------------- */

int gpio_config_input(unsigned int gpio)
{
    if (gpio_export(gpio) < 0)
        return GPIO_ERROR;

    if (gpio_set_direction(gpio,
                           GPIO_DIRECTION_IN) < 0)
        return GPIO_ERROR;

    return GPIO_SUCCESS;
}


/* ---------------------------------------------------------
 * Set GPIO value
 *
 * Equivalent:
 *
 * echo 1 > /sys/class/gpio/gpioX/value
 * --------------------------------------------------------- */

int gpio_set_value(unsigned int gpio,
                   unsigned int value)
{
    char value_string[4];

    if (value > 1) {
        fprintf(stderr,
                "Invalid GPIO value: %u\n",
                value);
        return GPIO_ERROR;
    }

    snprintf(value_string,
             sizeof(value_string),
             "%u",
             value);

    return gpio_sysfs_write(gpio,
                            "value",
                            value_string);
}


/* ---------------------------------------------------------
 * Get GPIO value
 * --------------------------------------------------------- */

int gpio_get_value(unsigned int gpio)
{
    char buffer[8];

    if (gpio_sysfs_read(gpio,
                        "value",
                        buffer,
                        sizeof(buffer)) < 0) {
        return GPIO_ERROR;
    }

    return atoi(buffer);
}


/* ---------------------------------------------------------
 * Read GPIO value as string
 * --------------------------------------------------------- */

int gpio_read_string(unsigned int gpio,
                     char *buffer,
                     size_t size)
{
    if (!buffer || size == 0)
        return GPIO_ERROR;

    return gpio_sysfs_read(gpio,
                           "value",
                           buffer,
                           size);
}


/* ---------------------------------------------------------
 * Toggle GPIO
 * --------------------------------------------------------- */

int gpio_toggle(unsigned int gpio)
{
    int current;
    int new_value;

    current = gpio_get_value(gpio);

    if (current < 0)
        return GPIO_ERROR;

    new_value = !current;

    if (gpio_set_value(gpio,
                       new_value) < 0)
        return GPIO_ERROR;

    return new_value;
}


/* ---------------------------------------------------------
 * Cleanup GPIO
 * --------------------------------------------------------- */

int gpio_cleanup(unsigned int gpio)
{
    return gpio_unexport(gpio);
}
