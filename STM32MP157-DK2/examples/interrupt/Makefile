# ==========================================================
# STM32MP157-DK2 GPIO Interrupt Examples
# ==========================================================

CC              ?= gcc

CFLAGS          := -Wall -Wextra -O2
CFLAGS          += -D_GNU_SOURCE

LIBS            := -lgpiod

TARGETS         := button_irq gpio_irq


# ==========================================================
# Default target
# ==========================================================

all: $(TARGETS)

	@echo ""
	@echo "=========================================="
	@echo " STM32MP157-DK2 Interrupt Examples"
	@echo " Build completed successfully"
	@echo "=========================================="
	@echo ""


# ==========================================================
# Button interrupt application
# ==========================================================

button_irq: button_irq.c

	@echo "Building button_irq..."

	$(CC) $(CFLAGS) \
		button_irq.c \
		-o button_irq \
		$(LIBS)


# ==========================================================
# Generic GPIO interrupt application
# ==========================================================

gpio_irq: gpio_irq.c

	@echo "Building gpio_irq..."

	$(CC) $(CFLAGS) \
		gpio_irq.c \
		-o gpio_irq \
		$(LIBS)


# ==========================================================
# Install
# ==========================================================

install: all

	@echo "Installing GPIO interrupt applications..."

	sudo install -m 0755 button_irq \
		/usr/local/bin/button_irq

	sudo install -m 0755 gpio_irq \
		/usr/local/bin/gpio_irq

	@echo "Installation completed."


# ==========================================================
# Uninstall
# ==========================================================

uninstall:

	sudo rm -f /usr/local/bin/button_irq
	sudo rm -f /usr/local/bin/gpio_irq

	@echo "Applications removed."


# ==========================================================
# Clean
# ==========================================================

clean:

	rm -f button_irq
	rm -f gpio_irq

	@echo "Build files cleaned."


# ==========================================================
# Test
# ==========================================================

test: all

	@echo ""
	@echo "GPIO interrupt applications:"
	@echo ""
	@echo "  ./button_irq /dev/gpiochip0 14"
	@echo "  ./gpio_irq   /dev/gpiochip0 14"
	@echo ""


# ==========================================================
# Help
# ==========================================================

help:

	@echo ""
	@echo "STM32MP157-DK2 GPIO Interrupt Examples"
	@echo ""
	@echo "Targets:"
	@echo "  make          - Build all applications"
	@echo "  make clean    - Remove binaries"
	@echo "  make install  - Install applications"
	@echo "  make uninstall- Remove installed applications"
	@echo "  make test     - Display test commands"
	@echo "  make help     - Display this help"
	@echo ""
