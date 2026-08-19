# ============================================================
# STM32MP157-DK2 LED Examples Makefile
# ============================================================

CC ?= gcc

CFLAGS  = -Wall
CFLAGS += -Wextra
CFLAGS += -O2
CFLAGS += -I../../application/include

LDFLAGS = -lgpiod

# Application sources
COMMON_SRC = ../../application/src/gpio-common.c
LIBGPIOD_SRC = ../../application/src/gpio-libgpiod.c
SYSFS_SRC = ../../application/src/gpio-sysfs.c

# Targets
TARGETS = gpio_toggle led_blink

# Default target
all: $(TARGETS)

# ============================================================
# GPIO Toggle
# ============================================================

gpio_toggle: gpio_toggle.c $(COMMON_SRC) $(LIBGPIOD_SRC) $(SYSFS_SRC)
	$(CC) $(CFLAGS) \
		-o $@ \
		gpio_toggle.c \
		$(COMMON_SRC) \
		$(LIBGPIOD_SRC) \
		$(SYSFS_SRC) \
		$(LDFLAGS)

# ============================================================
# LED Blink
# ============================================================

led_blink: led_blink.c $(COMMON_SRC) $(LIBGPIOD_SRC) $(SYSFS_SRC)
	$(CC) $(CFLAGS) \
		-o $@ \
		led_blink.c \
		$(COMMON_SRC) \
		$(LIBGPIOD_SRC) \
		$(SYSFS_SRC) \
		$(LDFLAGS)

# ============================================================
# Build
# ============================================================

build: all

# ============================================================
# Clean
# ============================================================

clean:
	rm -f $(TARGETS)
	rm -f *.o

# ============================================================
# Install
# ============================================================

PREFIX ?= /usr/local/bin

install: all
	install -d $(PREFIX)
	install -m 0755 gpio_toggle $(PREFIX)/
	install -m 0755 led_blink $(PREFIX)/

# ============================================================
# Uninstall
# ============================================================

uninstall:
	rm -f $(PREFIX)/gpio_toggle
	rm -f $(PREFIX)/led_blink

# ============================================================
# Test
# ============================================================

test: all
	@echo "========================================"
	@echo " STM32MP157-DK2 LED Examples"
	@echo "========================================"
	@echo ""
	@echo "Built applications:"
	@echo "  ./gpio_toggle"
	@echo "  ./led_blink"
	@echo ""
	@echo "Run on STM32MP157-DK2:"
	@echo "  sudo ./gpio_toggle"
	@echo "  sudo ./led_blink"

# ============================================================
# Debug Build
# ============================================================

debug: CFLAGS += -g -DDEBUG
debug: clean all

# ============================================================
# Release Build
# ============================================================

release: CFLAGS += -DNDEBUG
release: clean all

.PHONY: all build clean install uninstall test debug release
