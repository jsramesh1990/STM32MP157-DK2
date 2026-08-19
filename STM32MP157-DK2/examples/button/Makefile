# ============================================================
# STM32MP157-DK2
# GPIO Button Example
# ============================================================

TARGET := button

CC ?= gcc

CFLAGS := -Wall \
          -Wextra \
          -Werror \
          -O2

LDFLAGS :=
LDLIBS := -lgpiod

SRC := button.c

OBJ := $(SRC:.c=.o)

PREFIX ?= /usr/local/bin


# ------------------------------------------------------------
# Default target
# ------------------------------------------------------------

all: $(TARGET)


# ------------------------------------------------------------
# Build application
# ------------------------------------------------------------

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)


# ------------------------------------------------------------
# Compile source
# ------------------------------------------------------------

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@


# ------------------------------------------------------------
# Install
# ------------------------------------------------------------

install: $(TARGET)
	install -d $(DESTDIR)$(PREFIX)
	install -m 0755 $(TARGET) $(DESTDIR)$(PREFIX)/$(TARGET)


# ------------------------------------------------------------
# Uninstall
# ------------------------------------------------------------

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/$(TARGET)


# ------------------------------------------------------------
# Clean
# ------------------------------------------------------------

clean:
	rm -f $(TARGET) $(OBJ)


# ------------------------------------------------------------
# Debug build
# ------------------------------------------------------------

debug: CFLAGS := -Wall -Wextra -g -O0

debug: clean $(TARGET)


# ------------------------------------------------------------
# Help
# ------------------------------------------------------------

help:
	@echo "STM32MP157-DK2 GPIO Button Example"
	@echo ""
	@echo "Targets:"
	@echo "  make          - Build button application"
	@echo "  make debug    - Build debug version"
	@echo "  make clean    - Remove build files"
	@echo "  make install  - Install application"
	@echo "  make uninstall- Remove application"
