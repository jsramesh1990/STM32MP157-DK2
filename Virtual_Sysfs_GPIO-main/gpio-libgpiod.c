// gpio-libgpiod.c
// Requires libgpiod (headers & library).
// Usage: ./gpio-libgpiod <chip> <line-offset> <get|set> [0|1]
// Example: ./gpio-libgpiod /dev/gpiochip0 17 set 1

#include <gpiod.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
    if (argc < 4) {
        fprintf(stderr, "Usage: %s <chip> <line> <get|set> [0|1]\n", argv[0]);
        return 2;
    }
    const char *chipname = argv[1];
    unsigned int line = atoi(argv[2]);
    const char *cmd = argv[3];

    struct gpiod_chip *chip = gpiod_chip_open(chipname);
    if (!chip) { perror("gpiod_chip_open"); return 1; }

    struct gpiod_line *gline = gpiod_chip_get_line(chip, line);
    if (!gline) { perror("gpiod_chip_get_line"); gpiod_chip_close(chip); return 1; }

    if (strcmp(cmd, "get") == 0) {
        if (gpiod_line_request_input(gline, "gpio-read") < 0) { perror("request_input"); goto cleanup; }
        int val = gpiod_line_get_value(gline);
        printf("%d\n", val);
    } else if (strcmp(cmd, "set") == 0) {
        if (argc < 5) { fprintf(stderr, "set requires value 0/1\n"); goto cleanup; }
        int v = atoi(argv[4]);
        if (gpiod_line_request_output(gline, "gpio-write", v) < 0) { perror("request_output"); goto cleanup; }
        // Already set by request_output; we can also call set_value.
        if (gpiod_line_set_value(gline, v) < 0) perror("set_value");
    } else {
        fprintf(stderr, "Unknown cmd\n");
    }

cleanup:
    gpiod_chip_close(chip);
    return 0;
}

