CC = gcc
CFLAGS = -Wall -O2

all: gpio-sysfs gpio-libgpiod

gpio-sysfs: gpio-sysfs.c
	$(CC) $(CFLAGS) -o $@ gpio-sysfs.c

gpio-libgpiod: gpio-libgpiod.c
	$(CC) $(CFLAGS) -o $@ gpio-libgpiod.c -lgpiod

clean:
	rm -f gpio-sysfs gpio-libgpiod

