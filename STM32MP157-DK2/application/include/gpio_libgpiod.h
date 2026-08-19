/*
 * gpio_libgpiod.h
 *
 * libgpiod GPIO interface
 * for STM32MP157-DK2.
 */

#ifndef GPIO_LIBGPIOD_H
#define GPIO_LIBGPIOD_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "gpio_common.h"

/*
 * libgpiod forward declarations.
 *
 * This avoids exposing libgpiod implementation
 * details to applications that only need the API.
 */
struct gpiod_chip;
struct gpiod_line;

/* ---------------------------------------------------------
 * GPIO libgpiod Context
 * --------------------------------------------------------- */

typedef struct {

    /*
     * GPIO chip:
     *
     * Example:
     * /dev/gpiochip0
     */
    struct gpiod_chip *chip;

    /*
     * GPIO line.
     */
    struct gpiod_line *line;

    /*
     * Chip number.
     */
    unsigned int chip_num;

    /*
     * Line offset.
     */
    unsigned int line_offset;

    /*
     * GPIO consumer name.
     */
    char consumer[64];

    /*
     * Requested direction.
     */
    gpio_direction_t direction;

    /*
     * Edge configuration.
     */
    gpio_edge_t edge;

    /*
     * Current value.
     */
    int value;

} gpio_libgpiod_t;

/* ---------------------------------------------------------
 * Chip APIs
 * --------------------------------------------------------- */

/*
 * Open GPIO chip.
 *
 * Example:
 *
 * gpio_gpiod_open_chip(0);
 *
 * opens:
 *
 * /dev/gpiochip0
 */
struct gpiod_chip *
gpio_gpiod_open_chip(unsigned int chip_num);

/*
 * Close GPIO chip.
 */
void gpio_gpiod_close_chip(struct gpiod_chip *chip);

/* ---------------------------------------------------------
 * GPIO Line APIs
 * --------------------------------------------------------- */

/*
 * Request GPIO line as output.
 */
int gpio_gpiod_request_output(
        gpio_libgpiod_t *gpio,
        int initial_value);

/*
 * Request GPIO line as input.
 */
int gpio_gpiod_request_input(
        gpio_libgpiod_t *gpio);

/*
 * Request GPIO line with edge detection.
 */
int gpio_gpiod_request_edge(
        gpio_libgpiod_t *gpio,
        gpio_edge_t edge);

/*
 * Release GPIO line.
 */
void gpio_gpiod_release(
        gpio_libgpiod_t *gpio);

/* ---------------------------------------------------------
 * GPIO Value APIs
 * --------------------------------------------------------- */

/*
 * Set GPIO value.
 */
int gpio_gpiod_set_value(
        gpio_libgpiod_t *gpio,
        int value);

/*
 * Get GPIO value.
 */
int gpio_gpiod_get_value(
        gpio_libgpiod_t *gpio);

/*
 * Toggle GPIO.
 */
int gpio_gpiod_toggle(
        gpio_libgpiod_t *gpio);

/* ---------------------------------------------------------
 * GPIO Event APIs
 * --------------------------------------------------------- */

/*
 * Wait for GPIO edge event.
 *
 * timeout_ms:
 *
 *   < 0 : wait forever
 *    0  : non-blocking
 *   > 0 : timeout
 */
int gpio_gpiod_wait_event(
        gpio_libgpiod_t *gpio,
        int timeout_ms);

/*
 * Read GPIO edge event.
 *
 * Returns:
 *
 *  GPIO_EDGE_RISING
 *  GPIO_EDGE_FALLING
 */
int gpio_gpiod_read_event(
        gpio_libgpiod_t *gpio);

/* ---------------------------------------------------------
 * Information APIs
 * --------------------------------------------------------- */

/*
 * Print GPIO chip information.
 */
void gpio_gpiod_print_chip_info(
        struct gpiod_chip *chip);

/*
 * Print GPIO line information.
 */
void gpio_gpiod_print_line_info(
        struct gpiod_chip *chip,
        unsigned int line_offset);

#ifdef __cplusplus
}
#endif

#endif /* GPIO_LIBGPIOD_H */
