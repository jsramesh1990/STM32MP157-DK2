#include "gpio_common.h"

#include <time.h>
#include <ctype.h>


void gpio_print_config(const gpio_config_t *config)
{
    if (!config) {
        fprintf(stderr, "GPIO: invalid configuration\n");
        return;
    }

    printf("\n");
    printf("========================================\n");
    printf("        GPIO Configuration\n");
    printf("========================================\n");

    printf("Name       : %s\n",
           config->name[0] ? config->name : "unknown");

    printf("Pin        : %d\n", config->pin);
    printf("Chip       : %d\n", config->chip);
    printf("Line       : %d\n", config->line);

    printf("Direction  : %s\n", config->direction);
    printf("Value      : %d\n", config->value);
    printf("Edge       : %s\n", config->edge);
    printf("Bias       : %s\n", config->bias);

    printf("libgpiod   : %s\n",
           config->use_libgpiod ? "yes" : "no");

    printf("Simulation : %s\n",
           config->simulate ? "yes" : "no");

    printf("========================================\n");
}


int gpio_validate_value(int value)
{
    if (value != 0 && value != 1) {
        fprintf(stderr,
                "GPIO: invalid value %d. Expected 0 or 1\n",
                value);
        return -1;
    }

    return 0;
}


int gpio_parse_direction(const char *direction)
{
    if (!direction)
        return -1;

    if (strcmp(direction, "in") == 0)
        return 0;

    if (strcmp(direction, "out") == 0)
        return 1;

    if (strcmp(direction, "high") == 0)
        return 2;

    if (strcmp(direction, "low") == 0)
        return 3;

    fprintf(stderr,
            "GPIO: invalid direction '%s'\n",
            direction);

    return -1;
}


int gpio_parse_edge(const char *edge)
{
    if (!edge)
        return -1;

    if (strcmp(edge, "none") == 0)
        return 0;

    if (strcmp(edge, "rising") == 0)
        return 1;

    if (strcmp(edge, "falling") == 0)
        return 2;

    if (strcmp(edge, "both") == 0)
        return 3;

    fprintf(stderr,
            "GPIO: invalid edge '%s'\n",
            edge);

    return -1;
}


void gpio_sleep_ms(unsigned int ms)
{
    struct timespec ts;

    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (long)(ms % 1000) * 1000000L;

    while (nanosleep(&ts, &ts) < 0) {
        if (errno != EINTR)
            break;
    }
}


void gpio_print_error(const char *operation)
{
    if (!operation)
        operation = "GPIO operation";

    fprintf(stderr,
            "GPIO ERROR: %s: %s\n",
            operation,
            strerror(errno));
}
