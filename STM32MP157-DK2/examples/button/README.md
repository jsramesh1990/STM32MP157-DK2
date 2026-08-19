Build

On the STM32MP157-DK2 target:

cd STM32MP157-DK2/examples/button


make

Expected:

button
button.o

Run:

./button

Example output:

========================================
 STM32MP157-DK2 GPIO Button Test
========================================
GPIO Chip : /dev/gpiochip0
GPIO Line : 13
Press Ctrl+C to exit.


GPIO13 configured as INPUT
Waiting for button state changes...


GPIO13 = 1 -> BUTTON RELEASED
GPIO13 = 0 -> BUTTON PRESSED
GPIO13 = 1 -> BUTTON RELEASED
3. README.md
# STM32MP157-DK2 GPIO Button Example


## Overview


This example demonstrates how to read a physical button connected
to a GPIO pin on the STM32MP157-DK2 development board using the
Linux GPIO subsystem and libgpiod.


The application runs completely in userspace.


---


## Directory Structure


```text
button/
├── button.c
├── Makefile
└── README.md
Architecture
+-----------------------------+
|       Physical Button       |
+--------------+--------------+
               |
               v
+-----------------------------+
|     STM32MP157 GPIO        |
|       Controller            |
+--------------+--------------+
               |
               v
+-----------------------------+
|      Linux GPIO Driver      |
+--------------+--------------+
               |
               v
+-----------------------------+
|     /dev/gpiochip0          |
+--------------+--------------+
               |
               v
+-----------------------------+
|          libgpiod           |
+--------------+--------------+
               |
               v
+-----------------------------+
|         button.c            |
+-----------------------------+
Software Flow
Application Start
       |
       v
Open GPIO Chip
       |
       v
Get GPIO Line
       |
       v
Request GPIO as INPUT
       |
       v
Read GPIO Value
       |
       +------> Value = 0
       |           |
       |           v
       |      Button Pressed
       |
       +------> Value = 1
                   |
                   v
              Button Released
       |
       v
Continue Reading
       |
       v
Ctrl+C
       |
       v
Release GPIO
       |
       v
Close GPIO Chip
       |
       v
Exit
Requirements

The target Linux system must provide:

Linux GPIO subsystem
/dev/gpiochip*
libgpiod
C compiler
Make

Check GPIO devices:

ls -l /dev/gpiochip*

Example:

/dev/gpiochip0
/dev/gpiochip1
/dev/gpiochip2
Check GPIO Controllers

Use:

gpiodetect

Example:

gpiochip0 [gpiochip0] (16 lines)
gpiochip1 [gpiochip1] (16 lines)
gpiochip2 [gpiochip2] (16 lines)

Then inspect GPIO lines:

gpioinfo gpiochip0

Find the GPIO line connected to the button.

Important GPIO Concept

The physical board pin number and Linux GPIO line offset are
not necessarily the same.

For example:

Physical Pin
      |
      v
STM32 GPIO Port
      |
      v
GPIO Controller
      |
      v
Linux GPIO Line Offset

Therefore, verify the GPIO line before changing:

#define GPIO_CHIP "/dev/gpiochip0"
#define GPIO_LINE 13
Device Tree

The GPIO pin configuration should normally be described through
the Device Tree.

Typical configuration includes:

GPIO pin selection
GPIO direction
Pull-up / pull-down
Pin multiplexing
Interrupt configuration

Conceptually:

Device Tree
     |
     v
pinctrl configuration
     |
     v
STM32 GPIO Controller
     |
     v
/dev/gpiochipX
     |
     v
libgpiod
     |
     v
button.c
Build

Build using:

make

Expected output:

gcc -Wall -Wextra -Werror -O2 -c button.c -o button.o
gcc -Wall -Wextra -Werror -O2 -o button button.o -lgpiod

The executable will be:

./button
Run

Execute:

./button

Example:

========================================
 STM32MP157-DK2 GPIO Button Test
========================================
GPIO Chip : /dev/gpiochip0
GPIO Line : 13
Press Ctrl+C to exit.


GPIO13 configured as INPUT
Waiting for button state changes...


GPIO13 = 1 -> BUTTON RELEASED
GPIO13 = 0 -> BUTTON PRESSED
GPIO13 = 1 -> BUTTON RELEASED

Stop the program:

Ctrl+C
Button Logic

This example assumes an active-low button.

Button Released
       |
       v
GPIO = 1


Button Pressed
       |
       v
GPIO = 0

The logic can be represented as:

GPIO = 0
   |
   +----> BUTTON PRESSED


GPIO = 1
   |
   +----> BUTTON RELEASED

If your hardware uses active-high logic, reverse the interpretation
in button.c.

Polling

This example uses polling.

The application reads the GPIO every 100 ms:

usleep(100000);

Flow:

Read GPIO
   |
   v
Wait 100 ms
   |
   v
Read GPIO
   |
   v
Wait 100 ms
   |
   v
Repeat

Polling is simple and useful for basic GPIO testing.

For real interrupt-driven operation, use:

examples/interrupt/button_irq.c
libgpiod API Used

The example uses the following libgpiod APIs:

Open GPIO chip
gpiod_chip_open()
Get GPIO line
gpiod_chip_get_line()
Request GPIO input
gpiod_line_request_input()
Read GPIO
gpiod_line_get_value()
Release GPIO
gpiod_line_release()
Close GPIO chip
gpiod_chip_close()
Error Handling

The application checks errors during:

GPIO chip open
      |
      v
GPIO line access
      |
      v
GPIO input request
      |
      v
GPIO value read

If any operation fails, the application prints an error and
releases the resources already allocated.

Debugging

Check GPIO devices:

ls /dev/gpiochip*

Check GPIO controllers:

gpiodetect

Check GPIO lines:

gpioinfo gpiochip0

Check kernel GPIO information:

cat /sys/kernel/debug/gpio

Check kernel messages:

dmesg | grep -i gpio
Permission Issues

If the application reports:

Permission denied

check:

ls -l /dev/gpiochip0

For development testing, you may run:

sudo ./button

For production systems, configure the appropriate device
permissions instead of permanently running applications as root.

Testing
Test 1: GPIO Device
ls /dev/gpiochip*
Test 2: GPIO Controller
gpiodetect
Test 3: GPIO Line
gpioinfo gpiochip0
Test 4: Application
./button
Test 5: Button

Press and release the physical button.

Expected:

GPIO13 = 0 -> BUTTON PRESSED
GPIO13 = 1 -> BUTTON RELEASED
Troubleshooting
GPIO chip not found

Error:

Cannot open GPIO chip /dev/gpiochip0

Check:

ls /dev/gpiochip*

The GPIO chip may have a different number.

Update:

#define GPIO_CHIP "/dev/gpiochipX"
GPIO line unavailable

Error:

Cannot get GPIO line

Check:

gpioinfo gpiochip0

Verify the correct line offset.

GPIO already in use

Another kernel driver or application may already own the GPIO.

Check:

gpioinfo gpiochip0

Look for:

[used]

or a consumer name.

Button state is always 0

Check:

Button wiring
GPIO pin
Device Tree configuration
Pull-up/pull-down
Active-low configuration
Button state is always 1

Check:

Button wiring
GPIO pin
Pull-up/pull-down
GPIO configuration
Active-high/active-low logic
Relationship With Other Examples

The GPIO examples form a progression:

examples/
|
+-- led/
|    |
|    +-- led_blink.c
|    +-- gpio_toggle.c
|
+-- button/
|    |
|    +-- button.c
|
+-- interrupt/
|    |
|    +-- button_irq.c
|    +-- gpio_irq.c
|
+-- pwm/
     |
     +-- pwm_led.c
     +-- pwm_fade.c

Learning order:

1. LED
   |
   v
2. GPIO Toggle
   |
   v
3. Button Input
   |
   v
4. GPIO Interrupt
   |
   v
5. PWM
Project Integration

The button example uses the Linux GPIO subsystem directly through
libgpiod.

The complete STM32MP157-DK2 project architecture is:

                 STM32MP157-DK2
                        |
                        v
                  Device Tree
                        |
                        v
                Linux GPIO Driver
                        |
                        v
                GPIO Subsystem
                        |
                        v
                  /dev/gpiochipX
                        |
                        v
                     libgpiod
                        |
                        v
                Application Layer
                        |
        +---------------+---------------+
        |               |               |
        v               v               v
       LED           Button         Interrupt
Future Improvements

This example can later be extended with:

GPIO edge detection
Interrupt-based button handling
Software debounce
Multiple buttons
Multiple GPIO chips
Configuration through JSON
Common GPIO abstraction layer
Sysfs fallback
Systemd service
Yocto package integration
License

MIT License



### Result


Your directory will now be:


```text
STM32MP157-DK2/
└── examples/
    └── button/
        ├── button.c
        ├── Makefile
        └── README.md

And this example specifically demonstrates GPIO input using Linux libgpiod on the STM32MP157-DK2, while the interrupt functionality stays cleanly separated into examples/interrupt/.
