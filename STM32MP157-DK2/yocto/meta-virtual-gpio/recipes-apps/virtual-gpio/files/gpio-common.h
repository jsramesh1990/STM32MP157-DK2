#ifndef GPIO_COMMON_H
#define GPIO_COMMON_H

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

/*
 * Generic GPIO configuration.
 *
 * STM32MP157 GPIO numbering is not necessarily the same
 * as the physical connector pin number.
 *
 * For libgpiod:
 *     chip = gpiochipX
 *     line = GPIO line offset
 *
 * For sysfs:
 *     pin = Linux global GPIO number
 */
typedef struct {
    int pin;
    int chip;
    int line;

    char direction[16];
    int value;

    char edge[16];
    char bias[16];

    int use_libgpiod;
    int simulate;

    char name[64];
} gpio_config_t;


/*
 * Common helper functions.
 */
void gpio_print_config(const gpio_config_t *config);

int gpio_validate_value(int value);

int gpio_parse_direction(const char *direction);

int gpio_parse_edge(const char *edge);

void gpio_sleep_ms(unsigned int ms);

void gpio_print_error(const char *operation);

#endif /* GPIO_COMMON_H */
