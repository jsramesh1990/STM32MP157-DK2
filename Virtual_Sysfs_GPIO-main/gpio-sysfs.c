// gpio-sysfs.c
// Simple user-space GPIO using /sys/class/gpio for learning & testing.
// Usage: ./gpio-sysfs <export|unexport|write|read> <pin> [value]
// Example: ./gpio-sysfs export 17
//          ./gpio-sysfs write 17 1
//          ./gpio-sysfs read 17

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

static int write_file(const char *path, const char *value) {
    int fd = open(path, O_WRONLY);
    if (fd < 0) return -1;
    ssize_t n = write(fd, value, strlen(value));
    close(fd);
    return (n == (ssize_t)strlen(value)) ? 0 : -1;
}

static int read_file(const char *path, char *buf, size_t bufsize) {
    int fd = open(path, O_RDONLY);
    if (fd < 0) return -1;
    ssize_t n = read(fd, buf, bufsize-1);
    close(fd);
    if (n<0) return -1;
    buf[n] = '\0';
    return 0;
}

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <export|unexport|write|read> <pin> [value]\n", argv[0]);
        return 2;
    }
    char *cmd = argv[1];
    char *pin = argv[2];
    char path[256], buf[32];

    if (strcmp(cmd, "export") == 0) {
        return write_file("/sys/class/gpio/export", pin);
    } else if (strcmp(cmd, "unexport") == 0) {
        return write_file("/sys/class/gpio/unexport", pin);
    } else if (strcmp(cmd, "write") == 0) {
        if (argc < 4) { fprintf(stderr,"write needs value\n"); return 2; }
        snprintf(path, sizeof(path), "/sys/class/gpio/gpio%s/direction", pin);
        if (access(path, F_OK) != 0) { // try to export first
            write_file("/sys/class/gpio/export", pin);
            usleep(100000);
        }
        write_file(path, "out");
        snprintf(path, sizeof(path), "/sys/class/gpio/gpio%s/value", pin);
        return write_file(path, argv[3]);
    } else if (strcmp(cmd, "read") == 0) {
        snprintf(path, sizeof(path), "/sys/class/gpio/gpio%s/value", pin);
        if (read_file(path, buf, sizeof(buf)) == 0) {
            printf("%s", buf);
            return 0;
        } else {
            fprintf(stderr, "Failed reading %s\n", path);
            return 1;
        }
    } else {
        fprintf(stderr, "Unknown command\n");
        return 2;
    }
}

