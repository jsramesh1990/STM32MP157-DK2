Important: libgpiod Version

The above examples use the libgpiod 2.x API.

On the STM32MP157-DK2 target, verify:

gpiodetect --version

and:

pkg-config --modversion libgpiod

Also:

ls /dev/gpiochip*

Then:

gpiodetect

Example:

gpiochip0 [gpio0] (16 lines)
gpiochip1 [gpio1] (16 lines)
gpiochip2 [gpio2] (16 lines)
...

Don't assume gpiochip0 line 5 is the physical button. The actual chip/line must come from the STM32MP157-DK2 board's GPIO/pin mapping and Device Tree.

4. Makefile for These Two Files

Since you have:

STM32MP157-DK2/application/examples/

I recommend adding:

STM32MP157-DK2/application/
├── Makefile
└── examples/
    ├── button.c
    └── button_irq.c

application/Makefile:

CC ?= gcc


CFLAGS  := -Wall -Wextra -O2
LDFLAGS := -lgpiod


TARGETS := button button_irq


EXAMPLE_DIR := examples


.PHONY: all clean install


all: $(TARGETS)


button: $(EXAMPLE_DIR)/button.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)


button_irq: $(EXAMPLE_DIR)/button_irq.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)


clean:
	rm -f $(TARGETS)


install: $(TARGETS)
	install -d $(DESTDIR)/usr/bin
	install -m 0755 button $(DESTDIR)/usr/bin/
	install -m 0755 button_irq $(DESTDIR)/usr/bin/

Build on the target:

cd application
make

You should get:

button
button_irq
5. Test button

First identify the GPIO:

gpiodetect

Then inspect it:

gpioinfo

Run:

./button /dev/gpiochip0 5

Output:

========================================
 STM32MP157-DK2 GPIO Button Example
========================================
GPIO Chip : /dev/gpiochip0
GPIO Line : 5


GPIO configured successfully.
Button monitoring started.
Press Ctrl+C to exit.


Initial state: RELEASED

Press the button:

[BUTTON] PRESSED

Release:

[BUTTON] RELEASED
6. Test button_irq

Run:

./button_irq /dev/gpiochip0 5

Output:

========================================
 STM32MP157-DK2 GPIO IRQ Example
========================================
GPIO Chip : /dev/gpiochip0
GPIO Line : 5
Debounce  : 50 ms


GPIO interrupt/event configuration successful.


Waiting for button events...
Press Ctrl+C to exit.

Button press:

[GPIO IRQ] FALLING EDGE
           BUTTON PRESSED

Button release:

[GPIO IRQ] RISING EDGE
           BUTTON RELEASED
7. Where These Fit in Your Project

Your application layer should now become:

application/
│
├── Makefile
│
├── common/
│   ├── gpio_common.c
│   └── gpio_common.h
│
├── gpio-control/
│   ├── gpio_control.c
│   └── gpio_control.h
│
└── examples/
    ├── button.c
    ├── button_irq.c
    ├── led.c
    ├── led_blink.c
    └── gpio_test.c

And the final hardware/software flow is:

                 STM32MP157-DK2
                        │
                        ▼
                   GPIO Pin
                        │
                        ▼
                 STM32 GPIO HW
                        │
                        ▼
                 Linux GPIO Driver
                        │
                        ▼
                    gpiolib
                        │
                        ▼
                 /dev/gpiochipX
                        │
                        ▼
                    libgpiod
                   ┌────┴─────┐
                   │          │
                   ▼          ▼
               button.c   button_irq.c
                   │          │
                   ▼          ▼
                Polling     GPIO Events
                   │          │
                   └────┬─────┘
                        ▼
                  Application
                        │
                        ▼
                  Button Status

button.c = polling-based GPIO input example.
button_irq.c = event/interrupt-based GPIO input example.

For your STM32MP157-DK2 project, button_irq.c is particularly useful because it lets you demonstrate the complete GPIO → IRQ → Linux GPIO framework → libgpiod → userspace application path in an interview.






