#ifndef GPIO_SYSFS_H
#define GPIO_SYSFS_H

#include "gpio_common.h"

/*
 * Linux GPIO sysfs backend.
 *
 * NOTE:
 * GPIO sysfs is deprecated in modern Linux kernels.
 * This backend is retained for learning, legacy BSPs,
 * and comparison with the modern libgpiod interface.
 */

#define GPIO_SYSFS_ROOT "/sys/class/gpio"


/*
 * Export GPIO.
 */
int gpio_sysfs_export(int pin);


/*
 * Unexport GPIO.
 */
int gpio_sysfs_unexport(int pin);


/*
 * Set GPIO direction.
 *
 * direction:
 *     "in"
 *     "out"
 *     "high"
 *     "low"
 */
int gpio_sysfs_set_direction(int pin, const char *direction);


/*
 * Write GPIO value.
 */
int gpio_sysfs_write(int pin, int value);


/*
 * Read GPIO value.
 */
int gpio_sysfs_read(int pin);


/*
 * Configure edge detection.
 *
 * edge:
 *     "none"
 *     "rising"
 *     "falling"
 *     "both"
 */
int gpio_sysfs_set_edge(int pin, const char *edge);


/*
 * Get sysfs GPIO path.
 */
int gpio_sysfs_get_path(int pin,
                        const char *attribute,
                        char *path,
                        size_t path_size);


/*
 * Check whether GPIO is exported.
 */
int gpio_sysfs_is_exported(int pin);

#endif /* GPIO_SYSFS_H */
