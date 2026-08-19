#ifndef GPIO_LIBGPIOD_H
#define GPIO_LIBGPIOD_H

#include "gpio_common.h"

#include <gpiod.h>

/*
 * GPIO handle used by the libgpiod backend.
 */
typedef struct {
    struct gpiod_chip *chip;
    struct gpiod_line *line;

    int chip_number;
    unsigned int line_offset;

    int requested;
} gpio_gpiod_handle_t;


/*
 * Open GPIO chip.
 *
 * chip_number:
 *     GPIO chip number, for example 0 -> /dev/gpiochip0
 */
int gpio_gpiod_open(gpio_gpiod_handle_t *handle,
                    int chip_number,
                    unsigned int line_offset);


/*
 * Configure GPIO as output.
 */
int gpio_gpiod_request_output(gpio_gpiod_handle_t *handle,
                              const char *consumer,
                              int initial_value);


/*
 * Configure GPIO as input.
 */
int gpio_gpiod_request_input(gpio_gpiod_handle_t *handle,
                             const char *consumer);


/*
 * Configure GPIO input with pull-up.
 */
int gpio_gpiod_request_input_pullup(gpio_gpiod_handle_t *handle,
                                    const char *consumer);


/*
 * Configure GPIO input with pull-down.
 */
int gpio_gpiod_request_input_pulldown(gpio_gpiod_handle_t *handle,
                                      const char *consumer);


/*
 * Write GPIO value.
 */
int gpio_gpiod_write(gpio_gpiod_handle_t *handle, int value);


/*
 * Read GPIO value.
 */
int gpio_gpiod_read(gpio_gpiod_handle_t *handle);


/*
 * Wait for GPIO edge event.
 *
 * edge:
 *     "rising"
 *     "falling"
 *     "both"
 */
int gpio_gpiod_wait_event(gpio_gpiod_handle_t *handle,
                          const char *edge,
                          int timeout_ms);


/*
 * Release GPIO.
 */
void gpio_gpiod_close(gpio_gpiod_handle_t *handle);


/*
 * Display GPIO chip information.
 */
int gpio_gpiod_info(int chip_number);

#endif /* GPIO_LIBGPIOD_H */
