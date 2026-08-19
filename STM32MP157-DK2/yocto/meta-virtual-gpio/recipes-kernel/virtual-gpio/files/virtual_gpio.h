#ifndef _VIRTUAL_GPIO_H_
#define _VIRTUAL_GPIO_H_

#include <linux/gpio/driver.h>
#include <linux/spinlock.h>

#define VGPIO_NUM_LINES        32
#define VGPIO_NAME             "virtual-gpio"

/*
 * Virtual GPIO controller
 *
 * Each GPIO line contains:
 *   direction : input/output
 *   value     : 0/1
 *
 * The state is maintained in software.
 */
struct virtual_gpio {
	struct gpio_chip chip;

	spinlock_t lock;

	unsigned long direction;
	unsigned long value;

	unsigned int num_gpios;
};

#endif /* _VIRTUAL_GPIO_H_ */
