// SPDX-License-Identifier: GPL-2.0
/*
 * STM32MP157-DK2 Virtual GPIO Driver
 *
 * Purpose:
 *   Provides a software-based GPIO controller for testing and learning.
 *
 * Features:
 *   - Creates a virtual GPIO controller
 *   - Supports GPIO input/output
 *   - Supports get/set GPIO value
 *   - Supports direction configuration
 *   - Exposes GPIOs through Linux GPIO subsystem
 *   - Can be controlled using libgpiod/gpio tools
 *
 * Example:
 *
 *   gpioinfo
 *   gpiodetect
 *   gpioset gpiochipX 0=1
 *   gpioget gpiochipX 0
 *
 * This driver does NOT control physical STM32MP157 GPIO pins.
 * It provides a software GPIO provider for testing.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/gpio/driver.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/platform_device.h>

#include "virtual_gpio.h"

/*
 * Driver information
 */
#define DRIVER_NAME         "virtual_gpio"
#define DRIVER_VERSION      "1.0"

/*
 * Global virtual GPIO device
 */
static struct virtual_gpio_device *vgpio_dev;

/*
 * Protect GPIO state against concurrent access.
 */
static DEFINE_MUTEX(vgpio_lock);

/* ------------------------------------------------------------------------- */
/* GPIO CHIP CALLBACKS                                                       */
/* ------------------------------------------------------------------------- */

/*
 * Get GPIO value
 *
 * Called by Linux GPIO subsystem when a user/application requests
 * the current value of a GPIO.
 */
static int vgpio_get(struct gpio_chip *chip, unsigned int offset)
{
    struct virtual_gpio_device *dev;
    int value;

    dev = gpiochip_get_data(chip);

    if (!dev || offset >= dev->num_gpios)
        return -EINVAL;

    mutex_lock(&vgpio_lock);

    value = dev->gpios[offset].value ? 1 : 0;

    mutex_unlock(&vgpio_lock);

    pr_debug(DRIVER_NAME ": GPIO %u read = %d\n",
             offset, value);

    return value;
}

/*
 * Set GPIO value
 *
 * Called when user/application changes the GPIO output value.
 */
static void vgpio_set(struct gpio_chip *chip,
                      unsigned int offset,
                      int value)
{
    struct virtual_gpio_device *dev;

    dev = gpiochip_get_data(chip);

    if (!dev || offset >= dev->num_gpios)
        return;

    mutex_lock(&vgpio_lock);

    /*
     * Only update value if GPIO is configured as output.
     */
    if (dev->gpios[offset].direction_output)
        dev->gpios[offset].value = !!value;

    mutex_unlock(&vgpio_lock);

    pr_debug(DRIVER_NAME ": GPIO %u set = %d\n",
             offset, !!value);
}

/*
 * Configure GPIO as input.
 */
static int vgpio_direction_input(struct gpio_chip *chip,
                                 unsigned int offset)
{
    struct virtual_gpio_device *dev;

    dev = gpiochip_get_data(chip);

    if (!dev || offset >= dev->num_gpios)
        return -EINVAL;

    mutex_lock(&vgpio_lock);

    dev->gpios[offset].direction_output = false;

    mutex_unlock(&vgpio_lock);

    pr_debug(DRIVER_NAME ": GPIO %u configured as INPUT\n",
             offset);

    return 0;
}

/*
 * Configure GPIO as output.
 */
static int vgpio_direction_output(struct gpio_chip *chip,
                                  unsigned int offset,
                                  int value)
{
    struct virtual_gpio_device *dev;

    dev = gpiochip_get_data(chip);

    if (!dev || offset >= dev->num_gpios)
        return -EINVAL;

    mutex_lock(&vgpio_lock);

    dev->gpios[offset].direction_output = true;
    dev->gpios[offset].value = !!value;

    mutex_unlock(&vgpio_lock);

    pr_debug(DRIVER_NAME ": GPIO %u configured as OUTPUT, value=%d\n",
             offset, !!value);

    return 0;
}

/*
 * Request GPIO
 *
 * This callback is optional, but useful for debugging and
 * resource tracking.
 */
static int vgpio_request(struct gpio_chip *chip,
                         unsigned int offset)
{
    struct virtual_gpio_device *dev;

    dev = gpiochip_get_data(chip);

    if (!dev || offset >= dev->num_gpios)
        return -EINVAL;

    pr_debug(DRIVER_NAME ": GPIO %u requested\n", offset);

    return 0;
}

/*
 * Free GPIO
 */
static void vgpio_free(struct gpio_chip *chip,
                       unsigned int offset)
{
    pr_debug(DRIVER_NAME ": GPIO %u released\n", offset);
}

/* ------------------------------------------------------------------------- */
/* GPIO CHIP SETUP                                                           */
/* ------------------------------------------------------------------------- */

static struct gpio_chip vgpio_chip = {
    .label = DRIVER_NAME,
    .owner = THIS_MODULE,

    /*
     * Dynamic GPIO numbering.
     */
    .base = -1,

    .ngpio = VGPIO_NUM_GPIOS,

    /*
     * GPIO operations.
     */
    .request = vgpio_request,
    .free = vgpio_free,

    .get = vgpio_get,
    .set = vgpio_set,

    .direction_input = vgpio_direction_input,
    .direction_output = vgpio_direction_output,
};

/* ------------------------------------------------------------------------- */
/* MODULE INIT                                                               */
/* ------------------------------------------------------------------------- */

static int __init vgpio_init(void)
{
    int ret;
    unsigned int i;

    pr_info("\n");
    pr_info("=============================================\n");
    pr_info(" STM32MP157-DK2 Virtual GPIO Driver\n");
    pr_info(" Version: %s\n", DRIVER_VERSION);
    pr_info("=============================================\n");

    /*
     * Allocate virtual GPIO device.
     */
    vgpio_dev = kzalloc(sizeof(*vgpio_dev), GFP_KERNEL);

    if (!vgpio_dev) {
        pr_err(DRIVER_NAME ": failed to allocate device\n");
        return -ENOMEM;
    }

    /*
     * Number of GPIOs.
     */
    vgpio_dev->num_gpios = VGPIO_NUM_GPIOS;

    /*
     * Allocate GPIO state array.
     */
    vgpio_dev->gpios = kcalloc(
        vgpio_dev->num_gpios,
        sizeof(struct virtual_gpio),
        GFP_KERNEL
    );

    if (!vgpio_dev->gpios) {
        pr_err(DRIVER_NAME ": failed to allocate GPIO array\n");

        kfree(vgpio_dev);

        return -ENOMEM;
    }

    /*
     * Initialize every virtual GPIO.
     */
    for (i = 0; i < vgpio_dev->num_gpios; i++) {

        vgpio_dev->gpios[i].value = false;

        /*
         * Default direction = input.
         */
        vgpio_dev->gpios[i].direction_output = false;

        snprintf(vgpio_dev->gpios[i].label,
                 sizeof(vgpio_dev->gpios[i].label),
                 "virtual-gpio-%u",
                 i);
    }

    /*
     * Connect private driver data to GPIO chip.
     */
    gpiochip_set_data(&vgpio_chip, vgpio_dev);

    /*
     * Register GPIO controller with Linux GPIO subsystem.
     */
    ret = gpiochip_add_data(&vgpio_chip, vgpio_dev);

    if (ret) {
        pr_err(DRIVER_NAME
               ": gpiochip registration failed: %d\n",
               ret);

        kfree(vgpio_dev->gpios);
        kfree(vgpio_dev);

        return ret;
    }

    pr_info(DRIVER_NAME
            ": registered successfully\n");

    pr_info(DRIVER_NAME
            ": GPIO count = %u\n",
            vgpio_dev->num_gpios);

    pr_info(DRIVER_NAME
            ": GPIO controller available through Linux GPIO subsystem\n");

    pr_info(DRIVER_NAME
            ": use 'gpioinfo' or 'gpiodetect' to inspect it\n");

    return 0;
}

/* ------------------------------------------------------------------------- */
/* MODULE EXIT                                                               */
/* ------------------------------------------------------------------------- */

static void __exit vgpio_exit(void)
{
    /*
     * Remove GPIO controller from Linux GPIO subsystem.
     */
    gpiochip_remove(&vgpio_chip);

    /*
     * Free allocated memory.
     */
    if (vgpio_dev) {
        kfree(vgpio_dev->gpios);
        kfree(vgpio_dev);
        vgpio_dev = NULL;
    }

    pr_info(DRIVER_NAME ": unloaded\n");
}

module_init(vgpio_init);
module_exit(vgpio_exit);

/* ------------------------------------------------------------------------- */
/* MODULE INFORMATION                                                        */
/* ------------------------------------------------------------------------- */

MODULE_LICENSE("GPL");
MODULE_AUTHOR("STM32MP157-DK2 GPIO Project");
MODULE_DESCRIPTION(
    "Virtual GPIO controller driver for STM32MP157-DK2"
);
MODULE_VERSION(DRIVER_VERSION);
