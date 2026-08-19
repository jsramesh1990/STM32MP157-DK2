/*
 * gpio-libgpiod.c
 *
 * GPIO control implementation using libgpiod.
 *
 * Target:
 *     STM32MP157-DK2
 *
 * Interface:
 *     /dev/gpiochipX
 *
 * This implementation supports:
 *     - GPIO output
 *     - GPIO input
 *     - GPIO read
 *     - GPIO write
 *     - GPIO toggle
 *     - GPIO edge detection
 *     - GPIO bias configuration
 *
 * libgpiod v1.x API
 */

#include "gpio_libgpiod.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <time.h>


/* ---------------------------------------------------------
 * Internal GPIO context
 * --------------------------------------------------------- */

typedef struct
{
    struct gpiod_chip *chip;
    struct gpiod_line *line;

    unsigned int chip_num;
    unsigned int line_offset;

    int requested;
    int direction;
} gpio_gpiod_context_t;


/*
 * Direction states
 */
#define GPIO_DIR_UNKNOWN   0
#define GPIO_DIR_INPUT     1
#define GPIO_DIR_OUTPUT    2


/* ---------------------------------------------------------
 * Chip path helper
 * --------------------------------------------------------- */

static int build_chip_path(unsigned int chip_num,
                           char *path,
                           size_t size)
{
    if (!path || size == 0)
        return GPIO_ERROR;

    snprintf(path,
             size,
             "/dev/gpiochip%u",
             chip_num);

    return GPIO_SUCCESS;
}


/* ---------------------------------------------------------
 * Open GPIO chip
 * --------------------------------------------------------- */

struct gpiod_chip *gpiod_open_chip(const char *chip_num)
{
    char path[64];
    unsigned int number;
    char *endptr = NULL;

    if (!chip_num)
        return NULL;

    /*
     * Accept:
     *
     *     "0"
     *     "1"
     *     "2"
     *
     * or:
     *
     *     "/dev/gpiochip0"
     */
    if (strncmp(chip_num,
                "/dev/gpiochip",
                strlen("/dev/gpiochip")) == 0) {

        struct gpiod_chip *chip;

        chip = gpiod_chip_open(chip_num);

        if (!chip)
            perror("gpiod_chip_open");

        return chip;
    }

    number = strtoul(chip_num,
                     &endptr,
                     10);

    if (endptr == chip_num ||
        *endptr != '\0') {

        fprintf(stderr,
                "Invalid GPIO chip: %s\n",
                chip_num);

        return NULL;
    }

    if (build_chip_path(number,
                        path,
                        sizeof(path)) < 0)
        return NULL;

    return gpiod_chip_open(path);
}


/* ---------------------------------------------------------
 * Request GPIO line
 * --------------------------------------------------------- */

struct gpiod_line *gpiod_request_line(
        struct gpiod_chip *chip,
        unsigned int offset,
        struct gpiod_line_request_config *config)
{
    struct gpiod_line *line;

    if (!chip || !config)
        return NULL;

    line = gpiod_chip_get_line(chip,
                               offset);

    if (!line) {
        fprintf(stderr,
                "Failed to get GPIO line %u\n",
                offset);
        return NULL;
    }

    if (gpiod_line_request(line,
                           config,
                           0) < 0) {

        fprintf(stderr,
                "Failed to request GPIO line %u: %s\n",
                offset,
                strerror(errno));

        return NULL;
    }

    return line;
}


/* ---------------------------------------------------------
 * Release GPIO line
 * --------------------------------------------------------- */

static void gpiod_release_line(struct gpiod_line *line)
{
    if (!line)
        return;

    gpiod_line_release(line);
}


/* ---------------------------------------------------------
 * Request GPIO as output
 * --------------------------------------------------------- */

static int request_output(
        struct gpiod_line *line,
        const char *consumer,
        int value)
{
    int ret;

    if (!line)
        return GPIO_ERROR;

    ret = gpiod_line_request_output(
                line,
                consumer,
                value);

    if (ret < 0) {

        fprintf(stderr,
                "Failed to request GPIO output: %s\n",
                strerror(errno));

        return GPIO_ERROR;
    }

    return GPIO_SUCCESS;
}


/* ---------------------------------------------------------
 * Request GPIO as input
 * --------------------------------------------------------- */

static int request_input(
        struct gpiod_line *line,
        const char *consumer)
{
    int ret;

    if (!line)
        return GPIO_ERROR;

    ret = gpiod_line_request_input(
                line,
                consumer);

    if (ret < 0) {

        fprintf(stderr,
                "Failed to request GPIO input: %s\n",
                strerror(errno));

        return GPIO_ERROR;
    }

    return GPIO_SUCCESS;
}


/* ---------------------------------------------------------
 * Set GPIO line value
 * --------------------------------------------------------- */

int gpiod_line_set_value(
        struct gpiod_line *line,
        int value)
{
    if (!line)
        return GPIO_ERROR;

    if (value != 0 && value != 1) {
        fprintf(stderr,
                "Invalid GPIO value: %d\n",
                value);
        return GPIO_ERROR;
    }

    if (gpiod_line_set_value(
            line,
            value) < 0) {

        fprintf(stderr,
                "Failed to set GPIO value: %s\n",
                strerror(errno));

        return GPIO_ERROR;
    }

    return GPIO_SUCCESS;
}


/* ---------------------------------------------------------
 * Get GPIO line value
 * --------------------------------------------------------- */

int gpiod_line_get_value(
        struct gpiod_line *line)
{
    int value;

    if (!line)
        return GPIO_ERROR;

    value = gpiod_line_get_value(line);

    if (value < 0) {

        fprintf(stderr,
                "Failed to read GPIO value: %s\n",
                strerror(errno));

        return GPIO_ERROR;
    }

    return value;
}


/* ---------------------------------------------------------
 * Initialize GPIO context
 * --------------------------------------------------------- */

int gpio_gpiod_init(
        gpio_gpiod_context_t *ctx,
        unsigned int chip_num,
        unsigned int line_offset)
{
    char chip_path[64];

    if (!ctx)
        return GPIO_ERROR;

    memset(ctx,
           0,
           sizeof(*ctx));

    ctx->chip_num = chip_num;
    ctx->line_offset = line_offset;

    if (build_chip_path(chip_num,
                        chip_path,
                        sizeof(chip_path)) < 0)
        return GPIO_ERROR;

    ctx->chip = gpiod_chip_open(chip_path);

    if (!ctx->chip) {

        fprintf(stderr,
                "Failed to open %s: %s\n",
                chip_path,
                strerror(errno));

        return GPIO_ERROR;
    }

    ctx->line =
        gpiod_chip_get_line(ctx->chip,
                            line_offset);

    if (!ctx->line) {

        fprintf(stderr,
                "Failed to get GPIO line %u: %s\n",
                line_offset,
                strerror(errno));

        gpiod_chip_close(ctx->chip);

        ctx->chip = NULL;

        return GPIO_ERROR;
    }

    return GPIO_SUCCESS;
}


/* ---------------------------------------------------------
 * Configure GPIO output
 * --------------------------------------------------------- */

int gpio_gpiod_config_output(
        gpio_gpiod_context_t *ctx,
        int initial_value)
{
    if (!ctx || !ctx->line)
        return GPIO_ERROR;

    if (initial_value != 0 &&
        initial_value != 1)
        return GPIO_ERROR;

    /*
     * Release previous request if required.
     */
    if (ctx->requested) {

        gpiod_release_line(ctx->line);

        ctx->requested = 0;
    }

    if (request_output(ctx->line,
                       "stm32mp157-gpio",
                       initial_value) < 0)
        return GPIO_ERROR;

    ctx->requested = 1;
    ctx->direction = GPIO_DIR_OUTPUT;

    return GPIO_SUCCESS;
}


/* ---------------------------------------------------------
 * Configure GPIO input
 * --------------------------------------------------------- */

int gpio_gpiod_config_input(
        gpio_gpiod_context_t *ctx)
{
    if (!ctx || !ctx->line)
        return GPIO_ERROR;

    if (ctx->requested) {

        gpiod_release_line(ctx->line);

        ctx->requested = 0;
    }

    if (request_input(ctx->line,
                      "stm32mp157-gpio") < 0)
        return GPIO_ERROR;

    ctx->requested = 1;
    ctx->direction = GPIO_DIR_INPUT;

    return GPIO_SUCCESS;
}


/* ---------------------------------------------------------
 * Write GPIO
 * --------------------------------------------------------- */

int gpio_gpiod_write(
        gpio_gpiod_context_t *ctx,
        int value)
{
    if (!ctx ||
        !ctx->line ||
        !ctx->requested)
        return GPIO_ERROR;

    if (ctx->direction != GPIO_DIR_OUTPUT) {

        fprintf(stderr,
                "GPIO is not configured as output\n");

        return GPIO_ERROR;
    }

    return gpiod_line_set_value(ctx->line,
                                value);
}


/* ---------------------------------------------------------
 * Read GPIO
 * --------------------------------------------------------- */

int gpio_gpiod_read(
        gpio_gpiod_context_t *ctx)
{
    if (!ctx ||
        !ctx->line ||
        !ctx->requested)
        return GPIO_ERROR;

    if (ctx->direction != GPIO_DIR_INPUT &&
        ctx->direction != GPIO_DIR_OUTPUT) {

        fprintf(stderr,
                "GPIO is not configured\n");

        return GPIO_ERROR;
    }

    return gpiod_line_get_value(ctx->line);
}


/* ---------------------------------------------------------
 * Toggle GPIO
 * --------------------------------------------------------- */

int gpio_gpiod_toggle(
        gpio_gpiod_context_t *ctx)
{
    int current;
    int new_value;

    if (!ctx ||
        !ctx->line)
        return GPIO_ERROR;

    if (ctx->direction != GPIO_DIR_OUTPUT) {

        fprintf(stderr,
                "GPIO must be configured as output\n");

        return GPIO_ERROR;
    }

    current = gpio_gpiod_read(ctx);

    if (current < 0)
        return GPIO_ERROR;

    new_value = !current;

    if (gpio_gpiod_write(ctx,
                         new_value) < 0)
        return GPIO_ERROR;

    return new_value;
}


/* ---------------------------------------------------------
 * Configure rising-edge interrupt
 * --------------------------------------------------------- */

int gpio_gpiod_request_rising_edge(
        gpio_gpiod_context_t *ctx)
{
    if (!ctx || !ctx->line)
        return GPIO_ERROR;

    if (ctx->requested) {

        gpiod_release_line(ctx->line);

        ctx->requested = 0;
    }

    if (gpiod_line_request_rising_edge_events(
            ctx->line,
            "stm32mp157-button") < 0) {

        fprintf(stderr,
                "Failed to configure rising edge: %s\n",
                strerror(errno));

        return GPIO_ERROR;
    }

    ctx->requested = 1;
    ctx->direction = GPIO_DIR_INPUT;

    return GPIO_SUCCESS;
}


/* ---------------------------------------------------------
 * Configure falling-edge interrupt
 * --------------------------------------------------------- */

int gpio_gpiod_request_falling_edge(
        gpio_gpiod_context_t *ctx)
{
    if (!ctx || !ctx->line)
        return GPIO_ERROR;

    if (ctx->requested) {

        gpiod_release_line(ctx->line);

        ctx->requested = 0;
    }

    if (gpiod_line_request_falling_edge_events(
            ctx->line,
            "stm32mp157-button") < 0) {

        fprintf(stderr,
                "Failed to configure falling edge: %s\n",
                strerror(errno));

        return GPIO_ERROR;
    }

    ctx->requested = 1;
    ctx->direction = GPIO_DIR_INPUT;

    return GPIO_SUCCESS;
}


/* ---------------------------------------------------------
 * Configure both-edge interrupt
 * --------------------------------------------------------- */

int gpio_gpiod_request_both_edges(
        gpio_gpiod_context_t *ctx)
{
    if (!ctx || !ctx->line)
        return GPIO_ERROR;

    if (ctx->requested) {

        gpiod_release_line(ctx->line);

        ctx->requested = 0;
    }

    if (gpiod_line_request_both_edges_events(
            ctx->line,
            "stm32mp157-button") < 0) {

        fprintf(stderr,
                "Failed to configure both-edge events: %s\n",
                strerror(errno));

        return GPIO_ERROR;
    }

    ctx->requested = 1;
    ctx->direction = GPIO_DIR_INPUT;

    return GPIO_SUCCESS;
}


/* ---------------------------------------------------------
 * Wait for GPIO event
 * --------------------------------------------------------- */

int gpio_gpiod_wait_event(
        gpio_gpiod_context_t *ctx,
        unsigned int timeout_ms)
{
    struct timespec timeout;
    int ret;

    if (!ctx ||
        !ctx->line ||
        !ctx->requested)
        return GPIO_ERROR;

    timeout.tv_sec =
        timeout_ms / 1000;

    timeout.tv_nsec =
        (timeout_ms % 1000) * 1000000;

    ret = gpiod_line_event_wait(
                ctx->line,
                &timeout);

    if (ret < 0) {

        fprintf(stderr,
                "GPIO event wait failed: %s\n",
                strerror(errno));

        return GPIO_ERROR;
    }

    /*
     * 0 = timeout
     * 1 = event available
     */
    return ret;
}


/* ---------------------------------------------------------
 * Read GPIO event
 * --------------------------------------------------------- */

int gpio_gpiod_read_event(
        gpio_gpiod_context_t *ctx,
        struct gpiod_line_event *event)
{
    if (!ctx ||
        !ctx->line ||
        !event)
        return GPIO_ERROR;

    if (gpiod_line_event_read(
            ctx->line,
            event) < 0) {

        fprintf(stderr,
                "Failed to read GPIO event: %s\n",
                strerror(errno));

        return GPIO_ERROR;
    }

    return GPIO_SUCCESS;
}


/* ---------------------------------------------------------
 * Release GPIO line
 * --------------------------------------------------------- */

int gpio_gpiod_release(
        gpio_gpiod_context_t *ctx)
{
    if (!ctx)
        return GPIO_ERROR;

    if (ctx->line &&
        ctx->requested) {

        gpiod_release_line(ctx->line);

        ctx->requested = 0;
    }

    if (ctx->chip) {

        gpiod_chip_close(ctx->chip);

        ctx->chip = NULL;
    }

    ctx->line = NULL;
    ctx->direction = GPIO_DIR_UNKNOWN;

    return GPIO_SUCCESS;
}
