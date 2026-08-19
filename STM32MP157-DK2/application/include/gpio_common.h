/*
 * gpio_common.h
 *
 * Common GPIO definitions and APIs
 * for STM32MP157-DK2 GPIO application.
 */

#ifndef GPIO_COMMON_H
#define GPIO_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

/* ---------------------------------------------------------
 * GPIO Constants
 * --------------------------------------------------------- */

#define GPIO_SUCCESS        0
#define GPIO_ERROR         -1

#define GPIO_LOW            0
#define GPIO_HIGH           1

#define GPIO_INPUT          0
#define GPIO_OUTPUT         1

/* ---------------------------------------------------------
 * GPIO Interface
 * --------------------------------------------------------- */

typedef enum {
    GPIO_INTERFACE_SYSFS = 0,
    GPIO_INTERFACE_LIBGPIOD,
    GPIO_INTERFACE_SIMULATION
} gpio_interface_t;

/* ---------------------------------------------------------
 * GPIO Direction
 * --------------------------------------------------------- */

typedef enum {
    GPIO_DIR_INPUT = 0,
    GPIO_DIR_OUTPUT
} gpio_direction_t;

/* ---------------------------------------------------------
 * GPIO Edge
 * --------------------------------------------------------- */

typedef enum {
    GPIO_EDGE_NONE = 0,
    GPIO_EDGE_RISING,
    GPIO_EDGE_FALLING,
    GPIO_EDGE_BOTH
} gpio_edge_t;

/* ---------------------------------------------------------
 * GPIO Configuration
 * --------------------------------------------------------- */

typedef struct {

    /* Linux GPIO number / line offset */
    unsigned int gpio;

    /* GPIO chip number */
    unsigned int chip;

    /* Direction */
    gpio_direction_t direction;

    /* Initial value */
    int value;

    /* GPIO interface */
    gpio_interface_t interface;

    /* Edge configuration */
    gpio_edge_t edge;

    /* GPIO label */
    char consumer[64];

} gpio_config_t;

/* ---------------------------------------------------------
 * Common GPIO APIs
 * --------------------------------------------------------- */

/*
 * Initialize GPIO.
 */
int gpio_init(gpio_config_t *config);

/*
 * Configure GPIO direction.
 */
int gpio_set_direction(gpio_config_t *config,
                       gpio_direction_t direction);

/*
 * Set GPIO output value.
 */
int gpio_write(gpio_config_t *config,
               int value);

/*
 * Read GPIO input/output value.
 */
int gpio_read(gpio_config_t *config);

/*
 * Toggle GPIO.
 */
int gpio_toggle(gpio_config_t *config);

/*
 * Release GPIO resources.
 */
int gpio_cleanup(gpio_config_t *config);

/*
 * Delay helper.
 */
void gpio_delay_ms(unsigned int milliseconds);

/*
 * Print GPIO configuration.
 */
void gpio_print_config(const gpio_config_t *config);

#ifdef __cplusplus
}
#endif

#endif /* GPIO_COMMON_H */
