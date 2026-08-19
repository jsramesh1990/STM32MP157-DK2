/*
 * virtual_gpio.c
 *
 * Virtual GPIO controller for STM32MP157-DK2
 *
 * This driver creates a software GPIO controller using
 * the Linux GPIO subsystem.
 *
 * GPIO lines:
 *     GPIO0  ... GPIO31
 *
 * Userspace can access the controller through:
 *
 *     /dev/gpiochipX
 *
 * and tools such as:
 *
 *     gpioinfo
 *     gpioget
 *     gpioset
 *
 * This is useful for:
 *     - GPIO driver development
 *     - GPIO application testing
 *     - libgpiod testing
 *     - CI testing
 *     - simulation
 *     - learning Linux GPIO subsystem
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/spinlock.h>
#include <linux/gpio/driver.h>

#include "virtual_gpio.h"

/* --------------------------------------------------------- */
/* Module information                                        */
/* --------------------------------------------------------- */

MODULE_LICENSE("GPL");
MODULE_AUTHOR("STM32MP157-DK2 Project");
MODULE_DESCRIPTION("Virtual GPIO controller for STM32MP157-DK2");
MODULE_VERSION("1.0");


/* --------------------------------------------------------- */
/* GPIO get                                                   */
/* --------------------------------------------------------- */

static int virtual_gpio_get(struct gpio_chip *chip,
			    unsigned int offset)
{
	struct virtual_gpio *vgpio;
	unsigned long flags;
	int value;

	vgpio = gpiochip_get_data(chip);

	if (!vgpio)
		return -ENODEV;

	if (offset >= vgpio->num_gpios)
		return -EINVAL;

	spin_lock_irqsave(&vgpio->lock, flags);

	value = test_bit(offset, &vgpio->value);

	spin_unlock_irqrestore(&vgpio->lock, flags);

	pr_debug("virtual_gpio: GET GPIO%u = %d\n",
		 offset, value);

	return value;
}


/* --------------------------------------------------------- */
/* GPIO set                                                   */
/* --------------------------------------------------------- */

static void virtual_gpio_set(struct gpio_chip *chip,
			     unsigned int offset,
			     int value)
{
	struct virtual_gpio *vgpio;
	unsigned long flags;

	vgpio = gpiochip_get_data(chip);

	if (!vgpio)
		return;

	if (offset >= vgpio->num_gpios)
		return;

	spin_lock_irqsave(&vgpio->lock, flags);

	if (value)
		set_bit(offset, &vgpio->value);
	else
		clear_bit(offset, &vgpio->value);

	spin_unlock_irqrestore(&vgpio->lock, flags);

	pr_debug("virtual_gpio: SET GPIO%u = %d\n",
		 offset, value);
}


/* --------------------------------------------------------- */
/* Direction input                                            */
/* --------------------------------------------------------- */

static int virtual_gpio_direction_input(struct gpio_chip *chip,
					unsigned int offset)
{
	struct virtual_gpio *vgpio;
	unsigned long flags;

	vgpio = gpiochip_get_data(chip);

	if (!vgpio)
		return -ENODEV;

	if (offset >= vgpio->num_gpios)
		return -EINVAL;

	spin_lock_irqsave(&vgpio->lock, flags);

	/*
	 * Clear direction bit.
	 *
	 * 0 = input
	 * 1 = output
	 */
	clear_bit(offset, &vgpio->direction);

	spin_unlock_irqrestore(&vgpio->lock, flags);

	pr_debug("virtual_gpio: GPIO%u configured as INPUT\n",
		 offset);

	return 0;
}


/* --------------------------------------------------------- */
/* Direction output                                           */
/* --------------------------------------------------------- */

static int virtual_gpio_direction_output(struct gpio_chip *chip,
					 unsigned int offset,
					 int value)
{
	struct virtual_gpio *vgpio;
	unsigned long flags;

	vgpio = gpiochip_get_data(chip);

	if (!vgpio)
		return -ENODEV;

	if (offset >= vgpio->num_gpios)
		return -EINVAL;

	spin_lock_irqsave(&vgpio->lock, flags);

	/*
	 * Set initial GPIO value.
	 */
	if (value)
		set_bit(offset, &vgpio->value);
	else
		clear_bit(offset, &vgpio->value);

	/*
	 * Set direction to output.
	 */
	set_bit(offset, &vgpio->direction);

	spin_unlock_irqrestore(&vgpio->lock, flags);

	pr_debug("virtual_gpio: GPIO%u configured as OUTPUT value=%d\n",
		 offset, value);

	return 0;
}


/* --------------------------------------------------------- */
/* Get direction                                              */
/* --------------------------------------------------------- */

static int virtual_gpio_get_direction(struct gpio_chip *chip,
				      unsigned int offset)
{
	struct virtual_gpio *vgpio;
	unsigned long flags;
	int direction;

	vgpio = gpiochip_get_data(chip);

	if (!vgpio)
		return -ENODEV;

	if (offset >= vgpio->num_gpios)
		return -EINVAL;

	spin_lock_irqsave(&vgpio->lock, flags);

	if (test_bit(offset, &vgpio->direction))
		direction = GPIO_LINE_DIRECTION_OUT;
	else
		direction = GPIO_LINE_DIRECTION_IN;

	spin_unlock_irqrestore(&vgpio->lock, flags);

	return direction;
}


/* --------------------------------------------------------- */
/* GPIO request                                               */
/* --------------------------------------------------------- */

static int virtual_gpio_request(struct gpio_chip *chip,
				unsigned int offset)
{
	struct virtual_gpio *vgpio;

	vgpio = gpiochip_get_data(chip);

	if (!vgpio)
		return -ENODEV;

	if (offset >= vgpio->num_gpios)
		return -EINVAL;

	pr_debug("virtual_gpio: REQUEST GPIO%u\n", offset);

	return 0;
}


/* --------------------------------------------------------- */
/* GPIO free                                                  */
/* --------------------------------------------------------- */

static void virtual_gpio_free(struct gpio_chip *chip,
			      unsigned int offset)
{
	pr_debug("virtual_gpio: FREE GPIO%u\n", offset);
}


/* --------------------------------------------------------- */
/* GPIO labels                                                */
/* --------------------------------------------------------- */

static const char *virtual_gpio_line_names[VGPIO_NUM_LINES] = {
	"VGPIO0",
	"VGPIO1",
	"VGPIO2",
	"VGPIO3",
	"VGPIO4",
	"VGPIO5",
	"VGPIO6",
	"VGPIO7",
	"VGPIO8",
	"VGPIO9",
	"VGPIO10",
	"VGPIO11",
	"VGPIO12",
	"VGPIO13",
	"VGPIO14",
	"VGPIO15",
	"VGPIO16",
	"VGPIO17",
	"VGPIO18",
	"VGPIO19",
	"VGPIO20",
	"VGPIO21",
	"VGPIO22",
	"VGPIO23",
	"VGPIO24",
	"VGPIO25",
	"VGPIO26",
	"VGPIO27",
	"VGPIO28",
	"VGPIO29",
	"VGPIO30",
	"VGPIO31",
};


/* --------------------------------------------------------- */
/* Driver initialization                                      */
/* --------------------------------------------------------- */

static int __init virtual_gpio_init(void)
{
	struct virtual_gpio *vgpio;
	int ret;

	pr_info("virtual_gpio: initializing\n");

	/*
	 * Allocate controller structure.
	 */
	vgpio = kzalloc(sizeof(*vgpio), GFP_KERNEL);

	if (!vgpio) {
		pr_err("virtual_gpio: memory allocation failed\n");
		return -ENOMEM;
	}

	/*
	 * Initialize lock.
	 */
	spin_lock_init(&vgpio->lock);

	vgpio->num_gpios = VGPIO_NUM_LINES;

	/*
	 * Initially:
	 *
	 * All GPIOs are inputs.
	 * All GPIO values are LOW.
	 */
	vgpio->direction = 0;
	vgpio->value = 0;

	/*
	 * Configure gpio_chip.
	 */
	vgpio->chip.label = VGPIO_NAME;

	vgpio->chip.owner = THIS_MODULE;

	vgpio->chip.request = virtual_gpio_request;
	vgpio->chip.free = virtual_gpio_free;

	vgpio->chip.get = virtual_gpio_get;
	vgpio->chip.set = virtual_gpio_set;

	vgpio->chip.direction_input =
		virtual_gpio_direction_input;

	vgpio->chip.direction_output =
		virtual_gpio_direction_output;

	vgpio->chip.get_direction =
		virtual_gpio_get_direction;

	vgpio->chip.base = -1;

	vgpio->chip.ngpio = vgpio->num_gpios;

	vgpio->chip.can_sleep = false;

	vgpio->chip.names = virtual_gpio_line_names;

	/*
	 * Register GPIO controller.
	 */
	ret = gpiochip_add_data(&vgpio->chip, vgpio);

	if (ret) {
		pr_err("virtual_gpio: gpiochip registration failed: %d\n",
		       ret);

		kfree(vgpio);

		return ret;
	}

	pr_info("virtual_gpio: registered successfully\n");
	pr_info("virtual_gpio: controller=%s GPIOs=%u\n",
		VGPIO_NAME,
		vgpio->num_gpios);

	return 0;
}


/* --------------------------------------------------------- */
/* Driver cleanup                                             */
/* --------------------------------------------------------- */

static void __exit virtual_gpio_exit(void)
{
	struct gpio_chip *chip;
	struct virtual_gpio *vgpio;

	/*
	 * gpiochip_find_by_name() is not used here because
	 * the driver owns the controller directly.
	 *
	 * gpiochip_find() is used to locate our controller.
	 */

	chip = gpiochip_find(VGPIO_NAME,
			     NULL);

	if (!chip) {
		pr_info("virtual_gpio: controller not found\n");
		return;
	}

	vgpio = gpiochip_get_data(chip);

	if (!vgpio)
		return;

	/*
	 * Remove GPIO controller.
	 */
	gpiochip_remove(chip);

	kfree(vgpio);

	pr_info("virtual_gpio: removed\n");
}


module_init(virtual_gpio_init);
module_exit(virtual_gpio_exit);
