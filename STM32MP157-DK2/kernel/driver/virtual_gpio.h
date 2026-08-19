/* SPDX-License-Identifier: GPL-2.0 */

#ifndef VIRTUAL_GPIO_H
#define VIRTUAL_GPIO_H

#include <linux/types.h>

/*
 * Number of virtual GPIOs exposed by the driver.
 */
#define VGPIO_NUM_GPIOS        32

/*
 * Default GPIO base.
 *
 * Modern kernels may dynamically allocate the GPIO base.
 * This value is retained for compatibility/configuration.
 */
#define VGPIO_DEFAULT_BASE     -1

/*
 * Virtual GPIO state.
 */
struct virtual_gpio {
    bool value;
    bool direction_output;
    char label[32];
};

/*
 * Virtual GPIO device.
 */
struct virtual_gpio_device {
    struct virtual_gpio *gpios;
    unsigned int num_gpios;
};

#endif /* VIRTUAL_GPIO_H */
