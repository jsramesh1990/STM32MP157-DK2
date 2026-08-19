#include "gpio_sysfs.h"

#include <limits.h>


static int write_file(const char *path, const char *value)
{
    int fd;
    ssize_t len;
    size_t size;

    if (!path || !value)
        return -1;

    fd = open(path, O_WRONLY);

    if (fd < 0) {
        gpio_print_error(path);
        return -1;
    }

    size = strlen(value);

    len = write(fd, value, size);

    if (len < 0) {
        gpio_print_error(path);
        close(fd);
        return -1;
    }

    close(fd);

    if ((size_t)len != size) {
        fprintf(stderr,
                "GPIO: incomplete write to %s\n",
                path);
        return -1;
    }

    return 0;
}


static int read_file(const char *path,
                     char *buffer,
                     size_t buffer_size)
{
    int fd;
    ssize_t len;

    if (!path || !buffer || buffer_size < 2)
        return -1;

    fd = open(path, O_RDONLY);

    if (fd < 0) {
        gpio_print_error(path);
        return -1;
    }

    len = read(fd, buffer, buffer_size - 1);

    if (len < 0) {
        gpio_print_error(path);
        close(fd);
        return -1;
    }

    buffer[len] = '\0';

    close(fd);

    return 0;
}


int gpio_sysfs_get_path(int pin,
                        const char *attribute,
                        char *path,
                        size_t path_size)
{
    int ret;

    if (pin < 0 || !attribute || !path)
        return -1;

    ret = snprintf(path,
                   path_size,
                   GPIO_SYSFS_ROOT "/gpio%d/%s",
                   pin,
                   attribute);

    if (ret < 0 || (size_t)ret >= path_size)
        return -1;

    return 0;
}


int gpio_sysfs_is_exported(int pin)
{
    char path[PATH_MAX];

    if (gpio_sysfs_get_path(pin,
                            "value",
                            path,
                            sizeof(path)) < 0)
        return 0;

    return access(path, F_OK) == 0;
}


int gpio_sysfs_export(int pin)
{
    char value[32];

    if (pin < 0) {
        fprintf(stderr,
                "GPIO: invalid GPIO number %d\n",
                pin);
        return -1;
    }

    if (gpio_sysfs_is_exported(pin))
        return 0;

    snprintf(value, sizeof(value), "%d", pin);

    return write_file(GPIO_SYSFS_ROOT "/export", value);
}


int gpio_sysfs_unexport(int pin)
{
    char value[32];

    if (pin < 0)
        return -1;

    if (!gpio_sysfs_is_exported(pin))
        return 0;

    snprintf(value, sizeof(value), "%d", pin);

    return write_file(GPIO_SYSFS_ROOT "/unexport", value);
}


int gpio_sysfs_set_direction(int pin,
                             const char *direction)
{
    char path[PATH_MAX];

    if (gpio_parse_direction(direction) < 0)
        return -1;

    if (gpio_sysfs_get_path(pin,
                            "direction",
                            path,
                            sizeof(path)) < 0)
        return -1;

    return write_file(path, direction);
}


int gpio_sysfs_write(int pin, int value)
{
    char path[PATH_MAX];
    char value_string[8];

    if (gpio_validate_value(value) < 0)
        return -1;

    if (gpio_sysfs_get_path(pin,
                            "value",
                            path,
                            sizeof(path)) < 0)
        return -1;

    snprintf(value_string,
             sizeof(value_string),
             "%d",
             value);

    return write_file(path, value_string);
}


int gpio_sysfs_read(int pin)
{
    char path[PATH_MAX];
    char buffer[16];

    if (gpio_sysfs_get_path(pin,
                            "value",
                            path,
                            sizeof(path)) < 0)
        return -1;

    if (read_file(path,
                  buffer,
                  sizeof(buffer)) < 0)
        return -1;

    return atoi(buffer);
}


int gpio_sysfs_set_edge(int pin,
                        const char *edge)
{
    char path[PATH_MAX];

    if (gpio_parse_edge(edge) < 0)
        return -1;

    if (gpio_sysfs_get_path(pin,
                            "edge",
                            path,
                            sizeof(path)) < 0)
        return -1;

    return write_file(path, edge);
}
