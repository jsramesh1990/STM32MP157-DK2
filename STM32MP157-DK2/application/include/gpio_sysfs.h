#ifndef GPIO_SYSFS_H
#define GPIO_SYSFS_H

/*
 * GPIO Sysfs Interface
 *
 * Legacy Linux GPIO interface used through:
 *   /sys/class/gpio/
 *
 * NOTE:
 * The GPIO sysfs interface is deprecated in modern Linux kernels.
 * This implementation is retained for learning and compatibility
 * with systems where CONFIG_GPIO_SYSFS is enabled.
 */

#include <stddef.h>

/* Sysfs GPIO base directory */
#define GPIO_SYSFS_PATH          "/sys/class/gpio"

/* Maximum GPIO number supported by this application */
#define GPIO_MAX_NUMBER          1024

/* GPIO direction strings */
#define GPIO_DIRECTION_IN        "in"
#define GPIO_DIRECTION_OUT       "out"
#define GPIO_DIRECTION_HIGH      "high"
#define GPIO_DIRECTION_LOW       "low"

/* GPIO values */
#define GPIO_LOW                 0
#define GPIO_HIGH                1

/* Return values */
#define GPIO_SUCCESS             0
#define GPIO_ERROR              -1

/*
 * Export GPIO to userspace.
 *
 * Example:
 *     gpio_export(14);
 *
 * Equivalent shell command:
 *     echo 14 > /sys/class/gpio/export
 */
int gpio_export(unsigned int gpio);

/*
 * Unexport GPIO from userspace.
 *
 * Example:
 *     gpio_unexport(14);
 *
 * Equivalent shell command:
 *     echo 14 > /sys/class/gpio/unexport
 */
int gpio_unexport(unsigned int gpio);

/*
 * Set GPIO direction.
 *
 * Supported values:
 *     "in"
 *     "out"
 *     "high"
 *     "low"
 *
 * Example:
 *     gpio_set_direction(14, "out");
 */
int gpio_set_direction(unsigned int gpio, const char *direction);

/*
 * Set GPIO output value.
 *
 * value:
 *     0 = LOW
 *     1 = HIGH
 */
int gpio_set_value(unsigned int gpio, unsigned int value);

/*
 * Read GPIO input/output value.
 *
 * Returns:
 *     0 = LOW
 *     1 = HIGH
 *    -1 = Error
 */
int gpio_get_value(unsigned int gpio);

/*
 * Check whether GPIO is already exported.
 *
 * Returns:
 *     1 = GPIO exported
 *     0 = GPIO not exported
 *    -1 = Error
 */
int gpio_is_exported(unsigned int gpio);

/*
 * Wait until GPIO sysfs value file becomes available.
 *
 * timeout_ms:
 *     Maximum time to wait in milliseconds.
 *
 * Returns:
 *      0 = GPIO became available
 *     -1 = Timeout/error
 */
int gpio_wait_ready(unsigned int gpio, unsigned int timeout_ms);

/*
 * Configure GPIO as output and initialize its value.
 *
 * Example:
 *     gpio_config_output(14, 0);
 */
int gpio_config_output(unsigned int gpio, unsigned int value);

/*
 * Configure GPIO as input.
 *
 * Example:
 *     gpio_config_input(15);
 */
int gpio_config_input(unsigned int gpio);

/*
 * Toggle GPIO output value.
 *
 * Returns:
 *     New GPIO value
 *     -1 on error
 */
int gpio_toggle(unsigned int gpio);

/*
 * Read GPIO value as a string.
 *
 * buffer:
 *     Destination buffer.
 *
 * size:
 *     Size of destination buffer.
 *
 * Returns:
 *      0 on success
 *     -1 on error
 */
int gpio_read_string(unsigned int gpio, char *buffer, size_t size);

/*
 * Write a string to a GPIO sysfs attribute.
 *
 * This is a lower-level helper used by the GPIO implementation.
 */
int gpio_sysfs_write(unsigned int gpio,
                     const char *attribute,
                     const char *value);

/*
 * Read a GPIO sysfs attribute.
 */
int gpio_sysfs_read(unsigned int gpio,
                    const char *attribute,
                    char *buffer,
                    size_t size);

/*
 * Cleanup GPIO.
 *
 * This normally unexports the GPIO.
 */
int gpio_cleanup(unsigned int gpio);

#endif /* GPIO_SYSFS_H */
