Important

If your gpio-libgpiod.c uses the libgpiod v2 API, the exact library/compiler requirements may differ from the older API. The Makefile above assumes your source is compatible with:

pkg-config --cflags --libs libgpiod

If so, a better production Makefile is:

GPIOD_CFLAGS := $(shell pkg-config --cflags libgpiod 2>/dev/null)
GPIOD_LIBS   := $(shell pkg-config --libs libgpiod 2>/dev/null)


CFLAGS += $(GPIOD_CFLAGS)
LDFLAGS += $(GPIOD_LIBS)

instead of hard-coding -lgpiod.

4. README.md
# STM32MP157-DK2 LED GPIO Examples


This directory contains LED and GPIO output examples for the
STM32MP157-DK2 embedded Linux platform.


The examples use the common GPIO abstraction implemented in:


```text
application/
├── include/
│   ├── gpio_common.h
│   ├── gpio_libgpiod.h
│   └── gpio_sysfs.h
│
└── src/
    ├── gpio-common.c
    ├── gpio-libgpiod.c
    └── gpio-sysfs.c
Directory Structure
examples/
└── led/
    ├── gpio_toggle.c
    ├── led_blink.c
    ├── Makefile
    └── README.md
1. Purpose

The purpose of these examples is to demonstrate GPIO output
control on the STM32MP157-DK2 board using Linux GPIO interfaces.

The examples demonstrate:

GPIO initialization
GPIO direction configuration
GPIO output control
GPIO HIGH/LOW operation
LED ON/OFF control
GPIO toggling
Timing using Linux userspace APIs
Signal handling
GPIO cleanup
libgpiod-based GPIO access
2. Hardware Platform
STM32MP157-DK2

The project targets the STM32MP157-DK2 development platform.

The board is based on the STM32MP157 MPU family.

The STM32MP157 contains:

STM32MP157
│
├── Cortex-A7
│   ├── Linux
│   └── Embedded applications
│
└── Cortex-M4
    └── Real-time applications

For this project, GPIO control is performed from the
Linux userspace running on the Cortex-A7 processor.

3. GPIO Architecture

The application does not directly access STM32 GPIO registers.

Instead, the control path is:

Application
     │
     ▼
gpio_common.c
     │
     ├───────────────┐
     │               │
     ▼               ▼
libgpiod          Sysfs
     │               │
     ▼               ▼
/dev/gpiochip*   /sys/class/gpio
     │               │
     └───────┬───────┘
             ▼
       Linux GPIO
        subsystem
             │
             ▼
      STM32 GPIO Driver
             │
             ▼
       STM32 GPIO HW
             │
             ▼
            LED
4. GPIO Interface

The project primarily uses the modern Linux GPIO character
device interface through libgpiod.

Example:

Application
     |
     v
gpio_write()
     |
     v
libgpiod
     |
     v
/dev/gpiochip0
     |
     v
STM32 GPIO controller
     |
     v
GPIO pin

The legacy Sysfs interface is also supported by the project's
GPIO abstraction.

5. GPIO Configuration

The example currently uses:

#define LED_GPIO 13

and:

#define GPIO_PIN 13

These values are examples.

The GPIO number must correspond to the GPIO configuration of
the STM32MP157-DK2 board.

Do not assume that GPIO number 13 corresponds to a physical
header pin.

Verify the actual GPIO mapping using:

gpiodetect

and:

gpioinfo
6. gpio_toggle

Source:

gpio_toggle.c

The program continuously toggles the GPIO.

Flow:

Start
  |
  v
Configure GPIO
  |
  v
gpio_init()
  |
  v
Configure OUTPUT
  |
  v
Set GPIO HIGH
  |
  v
500 ms delay
  |
  v
Set GPIO LOW
  |
  v
500 ms delay
  |
  v
Repeat
  |
  v
Ctrl+C
  |
  v
GPIO LOW
  |
  v
gpio_cleanup()
  |
  v
Exit

Run:

sudo ./gpio_toggle

Example output:

========================================
 STM32MP157-DK2 GPIO Toggle Example
========================================
GPIO: 13
Press Ctrl+C to stop


GPIO13 -> HIGH
GPIO13 -> LOW
GPIO13 -> HIGH
GPIO13 -> LOW

Stop:

Ctrl+C
7. led_blink

Source:

led_blink.c

This example provides a higher-level LED abstraction.

Instead of directly writing GPIO values:

gpio_write(&gpio, 1);
gpio_write(&gpio, 0);

the application uses:

led_on(&gpio);
led_off(&gpio);

Flow:

Application
     |
     v
Initialize GPIO
     |
     v
Configure OUTPUT
     |
     v
LED OFF
     |
     v
LED ON
     |
     v
500 ms
     |
     v
LED OFF
     |
     v
500 ms
     |
     v
Next cycle
     |
     v
10 cycles
     |
     v
Cleanup

Run:

sudo ./led_blink
8. Build

Enter the directory:

cd STM32MP157-DK2/examples/led

Build:

make

Expected output:

gcc ... -o gpio_toggle ...
gcc ... -o led_blink ...

Check:

ls -l

Expected:

gpio_toggle
led_blink
gpio_toggle.c
led_blink.c
Makefile
README.md
9. Clean

Remove binaries:

make clean
10. Debug Build

Build with debug symbols:

make debug

This enables:

-g
-DDEBUG

You can then debug using:

gdb ./gpio_toggle
11. Verify GPIO Hardware

Before running the application:

gpiodetect

Example:

gpiochip0 [gpio-0-STM32MP1] (16 lines)
gpiochip1 [gpio-1-STM32MP1] (32 lines)
gpiochip2 [gpio-2-STM32MP1] (16 lines)

Then:

gpioinfo

Look for the required GPIO line.

12. GPIO Permissions

Check:

ls -l /dev/gpiochip*

If access is denied, run:

sudo ./gpio_toggle

For production systems, configure an appropriate udev rule
instead of relying on sudo.

13. Device Tree

GPIO functionality depends on the Device Tree configuration.

The project's Device Tree files are located in:

device-tree/
├── README.md
├── stm32mp157-gpio-test.dts
└── stm32mp157-gpio-test-overlay.dts

The Device Tree describes:

GPIO controller
      |
      +-- GPIO pin
      |
      +-- pinctrl configuration
      |
      +-- pull-up/down
      |
      +-- drive configuration
      |
      +-- GPIO consumer

The GPIO must not be claimed by another device if the
application needs to control it directly.

14. Linux GPIO Flow

The complete Linux GPIO flow is:

STM32MP157 Hardware
        |
        v
GPIO Controller
        |
        v
Device Tree
        |
        v
STM32 GPIO Driver
        |
        v
Linux GPIO Subsystem
        |
        v
/dev/gpiochipX
        |
        v
libgpiod
        |
        v
gpio-libgpiod.c
        |
        v
gpio-common.c
        |
        v
led_blink.c
        |
        v
LED
15. Why Use gpio_common.c?

The application should not be tightly coupled to a
specific GPIO interface.

Instead:

Application
     |
     v
GPIO Common API
     |
     +--------> libgpiod
     |
     +--------> Sysfs

This makes the application portable.

For example:

gpio_init(&gpio);


gpio_write(&gpio, 1);


gpio_write(&gpio, 0);


gpio_cleanup(&gpio);

The application does not need to know whether the underlying
implementation uses libgpiod or Sysfs.

16. GPIO Configuration Structure

The application uses:

gpio_config_t

Example:

gpio_config_t gpio;


gpio.pin = 13;
gpio.chip = 0;
gpio.value = 0;
gpio.use_libgpiod = 1;

Direction:

snprintf(gpio.direction,
         sizeof(gpio.direction),
         "out");

Edge:

snprintf(gpio.edge,
         sizeof(gpio.edge),
         "none");

Bias:

snprintf(gpio.bias,
         sizeof(gpio.bias),
         "default");
17. GPIO API

The common API provides:

gpio_init()

Initialize the GPIO.

gpio_write()

Write HIGH/LOW.

gpio_read()

Read GPIO value.

gpio_cleanup()

Release GPIO resources.

Example:

gpio_init(&gpio);


gpio_write(&gpio, 1);


gpio_write(&gpio, 0);


gpio_cleanup(&gpio);
18. Testing

Build:

make

Check binaries:

file gpio_toggle
file led_blink

Run:

sudo ./gpio_toggle

Then:

sudo ./led_blink

Monitor kernel messages:

dmesg | tail -50

Check GPIO state:

gpioinfo
19. Debugging

Use:

gpiodetect

to detect GPIO controllers.

Use:

gpioinfo

to inspect GPIO lines.

Use:

dmesg | grep -i gpio

to inspect kernel GPIO messages.

Use:

ls -l /dev/gpiochip*

to verify GPIO character devices.

Use:

strace ./gpio_toggle

to observe userspace system calls.

Use:

gdb ./gpio_toggle

for source-level debugging.

20. Expected Result

When the application runs successfully:

GPIO LOW
    |
    v
LED OFF


GPIO HIGH
    |
    v
LED ON


GPIO LOW
    |
    v
LED OFF


Repeat

The physical LED should blink at approximately:

500 ms ON
500 ms OFF
21. Error Handling

The application checks the return value of:

gpio_init()
gpio_write()
gpio_cleanup()

Example:

if (gpio_init(&gpio) < 0) {
    fprintf(stderr,
            "GPIO initialization failed\n");


    return EXIT_FAILURE;
}

This prevents the application from continuing when GPIO
initialization fails.

22. Project Relationship

The LED examples are part of the larger:

STM32MP157-DK2
GPIO Control & Simulator System

Project.

Overall structure:

STM32MP157-DK2/
│
├── application/
│   ├── include/
│   └── src/
│
├── configs/
│
├── device-tree/
│
├── docs/
│
└── examples/
    │
    ├── button/
    │
    ├── interrupt/
    │
    ├── led/
    │   ├── gpio_toggle.c
    │   ├── led_blink.c
    │   ├── Makefile
    │   └── README.md
    │
    └── pwm/
23. Learning Objectives

These examples demonstrate the following embedded Linux
concepts:

Linux GPIO subsystem
Device Tree GPIO configuration
libgpiod
GPIO character device
Userspace GPIO control
GPIO output configuration
LED control
Linux system calls
Signal handling
Error handling
Resource cleanup
Modular GPIO APIs
Embedded Linux application development
STM32MP1 BSP integration
Hardware-to-userspace GPIO flow
24. Interview Explanation

A simple explanation for this project:

"I developed GPIO applications on the STM32MP157-DK2 using the Linux GPIO subsystem and libgpiod. I created a common GPIO abstraction layer so that applications such as LED blinking and GPIO toggling are independent of the underlying GPIO interface. The GPIO is configured through Device Tree, exposed by the STM32 GPIO driver through the Linux GPIO subsystem, and accessed from userspace through the GPIO character device."

25. Future Extensions

The LED example can be extended with:

LED
 |
 +-- GPIO toggle
 |
 +-- Blink
 |
 +-- PWM brightness
 |
 +-- Button control
 |
 +-- Interrupt
 |
 +-- Multiple LEDs
 |
 +-- Thread-based control
 |
 +-- Timer-based blinking
 |
 +-- Configuration from JSON
 |
 +-- MQTT control
 |
 +-- Systemd service

This makes the project useful for demonstrating complete
embedded Linux GPIO development on STM32MP157-DK2.



### Final directory


Your `led` directory is therefore:


```text
STM32MP157-DK2/
└── examples/
    └── led/
        ├── gpio_toggle.c      # Continuous GPIO HIGH/LOW
        ├── led_blink.c        # Application-level LED blinking
        ├── Makefile            # Builds both applications
        └── README.md           # Documentation

And the overall project flow is:

                    STM32MP157-DK2
                           │
                           ▼
                    Device Tree
                           │
                           ▼
                  STM32 GPIO Driver
                           │
                           ▼
                Linux GPIO Subsystem
                           │
                 ┌─────────┴─────────┐
                 ▼                   ▼
          /dev/gpiochipX       /sys/class/gpio
                 │                   │
                 ▼                   ▼
              libgpiod            Sysfs
                 │                   │
                 └─────────┬─────────┘
                           ▼
                    gpio-common.c
                           │
                           ▼
                  Common GPIO API
                           │
             ┌─────────────┴─────────────┐
             ▼                           ▼
       gpio_toggle.c               led_blink.c
             │                           │
             └─────────────┬─────────────┘
                           ▼
                         GPIO
                           │
                           ▼
                          LED

This gives you a clean separation between BSP/Device Tree → kernel GPIO driver → GPIO subsystem → GPIO library → common API → application examples, which is a much stronger project structure for an Embedded Linux/BSP interview.
