# ==========================================================
# STM32MP157-DK2 PWM Examples
# ==========================================================

CC ?= gcc

CFLAGS := -Wall -Wextra -O2
CFLAGS += -D_GNU_SOURCE

TARGETS := pwm_led pwm_fade


# ==========================================================
# Default
# ==========================================================

all: $(TARGETS)

	@echo ""
	@echo "=========================================="
	@echo " STM32MP157-DK2 PWM Examples"
	@echo " Build completed successfully"
	@echo "=========================================="
	@echo ""


# ==========================================================
# PWM LED
# ==========================================================

pwm_led: pwm_led.c

	@echo "Building pwm_led..."

	$(CC) $(CFLAGS) \
		pwm_led.c \
		-o pwm_led


# ==========================================================
# PWM Fade
# ==========================================================

pwm_fade: pwm_fade.c

	@echo "Building pwm_fade..."

	$(CC) $(CFLAGS) \
		pwm_fade.c \
		-o pwm_fade


# ==========================================================
# Clean
# ==========================================================

clean:

	rm -f pwm_led
	rm -f pwm_fade

	@echo "PWM build files cleaned."


# ==========================================================
# Install
# ==========================================================

install: all

	@echo "Installing PWM applications..."

	sudo install -m 0755 pwm_led \
		/usr/local/bin/pwm_led

	sudo install -m 0755 pwm_fade \
		/usr/local/bin/pwm_fade

	@echo "Installation completed."


# ==========================================================
# Uninstall
# ==========================================================

uninstall:

	sudo rm -f /usr/local/bin/pwm_led
	sudo rm -f /usr/local/bin/pwm_fade

	@echo "PWM applications removed."


# ==========================================================
# Test
# ==========================================================

test: all

	@echo ""
	@echo "PWM applications built:"
	@echo ""
	@echo "  ./pwm_led  0 0 1000000 500000"
	@echo "  ./pwm_fade 0 0"
	@echo ""


# ==========================================================
# Help
# ==========================================================

help:

	@echo ""
	@echo "STM32MP157-DK2 PWM Example"
	@echo ""
	@echo "Targets:"
	@echo "  make           Build all PWM examples"
	@echo "  make clean     Remove binaries"
	@echo "  make install   Install applications"
	@echo "  make uninstall Remove applications"
	@echo "  make test      Show test commands"
	@echo "  make help      Show help"
	@echo ""
