#include "gpio_libgpiod.h"

#include <poll.h>
#include <time.h>


static void gpio_gpiod_init_handle(gpio_gpiod_handle_t *handle)
{
    if (!handle)
        return;

    memset(handle, 0, sizeof(*handle));

    handle->chip = NULL;
    handle->line = NULL;
    handle->chip_number = -1;
    handle->line_offset = 0;
    handle->requested = 0;
}


int gpio_gpiod_open(gpio_gpiod_handle_t *handle,
                    int chip_number,
                    unsigned int line_offset)
{
    char chip_path[64];

    if (!handle)
        return -1;

    gpio_gpiod_init_handle(handle);

    snprintf(chip_path,
             sizeof(chip_path),
             "/dev/gpiochip%d",
             chip_number);

    printf("Opening GPIO chip: %s\n", chip_path);

    handle->chip = gpiod_chip_open(chip_path);

    if (!handle->chip) {
        perror("gpiod_chip_open");
        return -1;
    }

    handle->line =
        gpiod_chip_get_line(handle->chip, line_offset);

    if (!handle->line) {
        perror("gpiod_chip_get_line");
        gpiod_chip_close(handle->chip);
        handle->chip = NULL;
        return -1;
    }

    handle->chip_number = chip_number;
    handle->line_offset = line_offset;

    return 0;
}


int gpio_gpiod_request_output(gpio_gpiod_handle_t *handle,
                              const char *consumer,
                              int initial_value)
{
    int ret;

    if (!handle || !handle->line)
        return -1;

    if (gpio_validate_value(initial_value) < 0)
        return -1;

    ret = gpiod_line_request_output(handle->line,
                                    consumer ? consumer : "stm32mp157-gpio",
                                    initial_value);

    if (ret < 0) {
        perror("gpiod_line_request_output");
        return -1;
    }

    handle->requested = 1;

    return 0;
}


int gpio_gpiod_request_input(gpio_gpiod_handle_t *handle,
                             const char *consumer)
{
    int ret;

    if (!handle || !handle->line)
        return -1;

    ret = gpiod_line_request_input(
        handle->line,
        consumer ? consumer : "stm32mp157-gpio");

    if (ret < 0) {
        perror("gpiod_line_request_input");
        return -1;
    }

    handle->requested = 1;

    return 0;
}


int gpio_gpiod_request_input_pullup(gpio_gpiod_handle_t *handle,
                                    const char *consumer)
{
    int ret;

    if (!handle || !handle->line)
        return -1;

    ret = gpiod_line_request_input_flags(
        handle->line,
        consumer ? consumer : "stm32mp157-gpio",
        GPIOD_LINE_REQUEST_FLAG_BIAS_PULL_UP);

    if (ret < 0) {
        perror("gpiod_line_request_input_flags");
        return -1;
    }

    handle->requested = 1;

    return 0;
}


int gpio_gpiod_request_input_pulldown(gpio_gpiod_handle_t *handle,
                                      const char *consumer)
{
    int ret;

    if (!handle || !handle->line)
        return -1;

    ret = gpiod_line_request_input_flags(
        handle->line,
        consumer ? consumer : "stm32mp157-gpio",
        GPIOD_LINE_REQUEST_FLAG_BIAS_PULL_DOWN);

    if (ret < 0) {
        perror("gpiod_line_request_input_flags");
        return -1;
    }

    handle->requested = 1;

    return 0;
}


int gpio_gpiod_write(gpio_gpiod_handle_t *handle,
                     int value)
{
    if (!handle || !handle->line || !handle->requested)
        return -1;

    if (gpio_validate_value(value) < 0)
        return -1;

    if (gpiod_line_set_value(handle->line, value) < 0) {
        perror("gpiod_line_set_value");
        return -1;
    }

    return 0;
}


int gpio_gpiod_read(gpio_gpiod_handle_t *handle)
{
    int value;

    if (!handle || !handle->line || !handle->requested)
        return -1;

    value = gpiod_line_get_value(handle->line);

    if (value < 0) {
        perror("gpiod_line_get_value");
        return -1;
    }

    return value;
}


int gpio_gpiod_wait_event(gpio_gpiod_handle_t *handle,
                          const char *edge,
                          int timeout_ms)
{
    struct gpiod_line_event event;
    struct timespec timeout;
    int ret;

    if (!handle || !handle->line)
        return -1;

    if (!edge)
        return -1;

    if (strcmp(edge, "rising") == 0) {

        if (gpiod_line_request_rising_edge_events(
                handle->line,
                "stm32mp157-gpio") < 0) {
            perror("gpiod_line_request_rising_edge_events");
            return -1;
        }

    } else if (strcmp(edge, "falling") == 0) {

        if (gpiod_line_request_falling_edge_events(
                handle->line,
                "stm32mp157-gpio") < 0) {
            perror("gpiod_line_request_falling_edge_events");
            return -1;
        }

    } else if (strcmp(edge, "both") == 0) {

        if (gpiod_line_request_both_edges_events(
                handle->line,
                "stm32mp157-gpio") < 0) {
            perror("gpiod_line_request_both_edges_events");
            return -1;
        }

    } else {
        fprintf(stderr,
                "Unsupported edge type: %s\n",
                edge);
        return -1;
    }

    handle->requested = 1;

    timeout.tv_sec = timeout_ms / 1000;
    timeout.tv_nsec =
        (long)(timeout_ms % 1000) * 1000000L;

    ret = gpiod_line_event_wait(handle->line,
                                &timeout);

    if (ret < 0) {
        perror("gpiod_line_event_wait");
        return -1;
    }

    if (ret == 0)
        return 0;

    ret = gpiod_line_event_read(handle->line,
                                &event);

    if (ret < 0) {
        perror("gpiod_line_event_read");
        return -1;
    }

    if (event.event_type ==
        GPIOD_LINE_EVENT_RISING_EDGE) {

        printf("GPIO%d: RISING EDGE\n",
               handle->line_offset);

        return 1;
    }

    if (event.event_type ==
        GPIOD_LINE_EVENT_FALLING_EDGE) {

        printf("GPIO%d: FALLING EDGE\n",
               handle->line_offset);

        return 2;
    }

    return 0;
}


void gpio_gpiod_close(gpio_gpiod_handle_t *handle)
{
    if (!handle)
        return;

    if (handle->requested && handle->line) {
        gpiod_line_release(handle->line);
        handle->requested = 0;
    }

    if (handle->chip) {
        gpiod_chip_close(handle->chip);
        handle->chip = NULL;
    }

    handle->line = NULL;
}


int gpio_gpiod_info(int chip_number)
{
    struct gpiod_chip *chip;
    char chip_path[64];
    unsigned int num_lines;
    unsigned int i;

    snprintf(chip_path,
             sizeof(chip_path),
             "/dev/gpiochip%d",
             chip_number);

    chip = gpiod_chip_open(chip_path);

    if (!chip) {
        perror("gpiod_chip_open");
        return -1;
    }

    printf("\n");
    printf("========================================\n");
    printf("        GPIO Chip Information\n");
    printf("========================================\n");

    printf("Chip      : %s\n", chip_path);

    printf("Label     : %s\n",
           gpiod_chip_label(chip));

    num_lines = gpiod_chip_num_lines(chip);

    printf("GPIO lines: %u\n", num_lines);

    for (i = 0; i < num_lines; i++) {

        struct gpiod_line *line;
        const char *name;
        const char *consumer;

        line = gpiod_chip_get_line(chip, i);

        if (!line)
            continue;

        name = gpiod_line_name(line);
        consumer = gpiod_line_consumer(line);

        printf("Line %3u : name=%-20s consumer=%s\n",
               i,
               name ? name : "-",
               consumer ? consumer : "-");
    }

    printf("========================================\n");

    gpiod_chip_close(chip);

    return 0;
}
