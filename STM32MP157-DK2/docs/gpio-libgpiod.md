Yes. For your **STM32MP157-DK2** project, the next documentation file should be:

`STM32MP157-DK2/docs/gpio-libgpiod.md`

Below is a complete version aligned with the project structure you have been building.

````markdown
# GPIO libgpiod Interface - STM32MP157-DK2

## 1. Overview

This document describes GPIO control on the STM32MP157-DK2 using the
Linux `libgpiod` interface.

The project supports the modern Linux GPIO character-device interface:

    /dev/gpiochip0
    /dev/gpiochip1
    ...

Instead of directly accessing GPIO registers from user space, the
application communicates with the Linux GPIO subsystem through
`libgpiod`.

The overall flow is:

    Application
         |
         v
    gpio-common.c
         |
         v
    gpio-libgpiod.c
         |
         v
    libgpiod
         |
         v
    /dev/gpiochipX
         |
         v
    Linux GPIO Driver
         |
         v
    STM32 GPIO Controller
         |
         v
    Physical GPIO Pin


---

# 2. Why libgpiod?

Older Linux applications commonly used GPIO through:

    /sys/class/gpio/

This interface is called the GPIO Sysfs interface.

Modern Linux kernels use the GPIO character-device interface:

    /dev/gpiochipX

`libgpiod` provides a user-space API for communicating with these
GPIO character devices.

### Advantages

- Modern Linux GPIO interface
- Better resource ownership
- GPIO line request/release mechanism
- Input/output configuration
- GPIO event support
- Rising/falling edge detection
- Pull-up/pull-down configuration
- Better multi-process handling
- Suitable for embedded Linux applications


---

# 3. Project Integration

The STM32MP157-DK2 project contains the following GPIO components:

    STM32MP157-DK2/
    |
    +-- application/
    |   |
    |   +-- include/
    |   |   |
    |   |   +-- gpio_common.h
    |   |   +-- gpio_libgpiod.h
    |   |   +-- gpio_sysfs.h
    |   |
    |   +-- src/
    |       |
    |       +-- gpio-common.c
    |       +-- gpio-libgpiod.c
    |       +-- gpio-sysfs.c
    |
    +-- application/examples/
    |   |
    |   +-- gpio_toggle.c
    |   +-- led_blink.c
    |   +-- button.c
    |   +-- button_irq.c
    |
    +-- device-tree/
    |
    +-- configs/
    |
    +-- docs/
    |
    +-- Makefile


---

# 4. GPIO Architecture

The GPIO application is divided into three layers.

```text
+--------------------------------------------------+
|              Application Examples               |
|                                                  |
| gpio_toggle.c                                    |
| led_blink.c                                      |
| button.c                                         |
| button_irq.c                                     |
+-------------------------+------------------------+
                          |
                          v
+--------------------------------------------------+
|              Common GPIO Layer                  |
|                                                  |
| gpio-common.c                                    |
| gpio_common.h                                    |
+-------------------------+------------------------+
                          |
             +------------+-------------+
             |                          |
             v                          v
+-------------------------+   +-------------------+
| Sysfs GPIO Backend      |   | libgpiod Backend  |
|                         |   |                   |
| gpio-sysfs.c            |   | gpio-libgpiod.c  |
| gpio_sysfs.h            |   | gpio_libgpiod.h  |
+------------+------------+   +---------+---------+
             |                          |
             v                          v
     /sys/class/gpio             /dev/gpiochipX
             |                          |
             +------------+-------------+
                          |
                          v
                 Linux GPIO Framework
                          |
                          v
                 STM32 GPIO Driver
                          |
                          v
                 STM32MP157 GPIO HW
````

The recommended backend for this project is `libgpiod`.

---

# 5. Linux GPIO Character Device

After Linux boots on the STM32MP157-DK2, GPIO controllers are exposed
through character devices.

Check:

```bash
ls -l /dev/gpiochip*
```

Example:

```text
/dev/gpiochip0
/dev/gpiochip1
/dev/gpiochip2
/dev/gpiochip3
/dev/gpiochip4
```

The exact number and GPIO mapping depend on the STM32MP157 Linux
device-tree configuration and kernel version.

---

# 6. Install libgpiod

On the development machine:

```bash
sudo apt update
sudo apt install libgpiod-dev gpiod
```

For Yocto, add the required package to the image.

For example:

```bitbake
IMAGE_INSTALL:append = " libgpiod libgpiod-tools"
```

For development headers:

```bitbake
IMAGE_INSTALL:append = " libgpiod-dev"
```

The exact package names can depend on the Yocto release and layer
configuration.

---

# 7. Check GPIO Chips

After booting the STM32MP157-DK2:

```bash
gpiodetect
```

Example:

```text
gpiochip0 [gpiochip0] (16 lines)
gpiochip1 [gpiochip1] (16 lines)
gpiochip2 [gpiochip2] (16 lines)
...
```

To get detailed information:

```bash
gpioinfo
```

Or:

```bash
gpioinfo gpiochip0
```

Example:

```text
line   0: "GPIO0" unused input active-high
line   1: "GPIO1" unused input active-high
line   2: "GPIO2" unused input active-high
...
```

---

# 8. GPIO Chip and Line Concept

It is important to understand the difference between a GPIO chip and
a GPIO line.

```text
GPIO Controller
      |
      +-- GPIO line 0
      +-- GPIO line 1
      +-- GPIO line 2
      +-- GPIO line 3
      |
      +-- ...
      |
      +-- GPIO line N
```

`gpiochipX` identifies a GPIO controller.

The line offset identifies a GPIO within that controller.

For example:

```text
/dev/gpiochip0
        |
        +-- line 0
        +-- line 1
        +-- line 2
        +-- line 3
        |
        +-- line 15
```

The application should use the GPIO **chip + line offset** rather than
assuming a global Linux GPIO number.

---

# 9. GPIO Line Request

Before accessing a GPIO line, the application requests ownership.

Conceptually:

```text
Application
     |
     v
Open gpiochip
     |
     v
Find GPIO line
     |
     v
Request line
     |
     v
Configure direction
     |
     v
Read / Write GPIO
     |
     v
Release line
```

This prevents multiple applications from incorrectly controlling the
same GPIO.

---

# 10. Output GPIO Flow

For an LED output:

```text
led_blink.c
     |
     v
gpio-common.c
     |
     v
gpio-libgpiod.c
     |
     v
open gpiochip
     |
     v
request GPIO line
     |
     v
configure OUTPUT
     |
     v
set value = 1
     |
     v
STM32 GPIO controller
     |
     v
Physical GPIO
     |
     v
LED ON
```

To switch the LED OFF:

```text
Application
     |
     v
gpio_write(0)
     |
     v
libgpiod
     |
     v
GPIO line
     |
     v
STM32 GPIO
     |
     v
LED OFF
```

---

# 11. Input GPIO Flow

For a push button:

```text
Physical Button
       |
       v
STM32 GPIO Pin
       |
       v
STM32 GPIO Controller
       |
       v
Linux GPIO Driver
       |
       v
/dev/gpiochipX
       |
       v
libgpiod
       |
       v
button.c
       |
       v
Application
```

The application can read:

```text
0 -> button pressed
1 -> button released
```

depending on the electrical configuration.

---

# 12. GPIO Output Example

A basic libgpiod output operation conceptually looks like:

```c
struct gpiod_chip *chip;
struct gpiod_line *line;

chip = gpiod_chip_open("/dev/gpiochip0");

line = gpiod_chip_get_line(chip, GPIO_LINE);

gpiod_line_request_output(
    line,
    "stm32mp157-gpio-test",
    0
);

gpiod_line_set_value(line, 1);

gpiod_line_release(line);
gpiod_chip_close(chip);
```

The exact API depends on the libgpiod major version installed on the
target.

---

# 13. GPIO Input Example

Conceptually:

```c
struct gpiod_chip *chip;
struct gpiod_line *line;
int value;

chip = gpiod_chip_open("/dev/gpiochip0");

line = gpiod_chip_get_line(chip, GPIO_LINE);

gpiod_line_request_input(
    line,
    "stm32mp157-button"
);

value = gpiod_line_get_value(line);

printf("GPIO value = %d\n", value);

gpiod_line_release(line);
gpiod_chip_close(chip);
```

This allows the application to read the GPIO state.

---

# 14. Edge Detection

GPIO inputs can generate events when the signal changes.

Supported events:

```text
Rising Edge
     0 -> 1

Falling Edge
     1 -> 0

Both Edges
     0 -> 1
     1 -> 0
```

The application can request:

```text
GPIOHANDLE_REQUEST_EVENT_RISING_EDGE
GPIOHANDLE_REQUEST_EVENT_FALLING_EDGE
GPIOHANDLE_REQUEST_EVENT_BOTH_EDGES
```

depending on the libgpiod API version.

---

# 15. Button Interrupt Flow

The `button_irq.c` example uses GPIO edge events.

```text
             Physical Button
                    |
                    v
              GPIO Pin
                    |
                    v
          STM32 GPIO Controller
                    |
                    v
             GPIO Interrupt
                    |
                    v
           Linux GPIO Driver
                    |
                    v
             gpiochip device
                    |
                    v
                libgpiod
                    |
                    v
             button_irq.c
                    |
                    v
            Event detected
                    |
                    v
             Application
```

Example:

```text
Button pressed
      |
      v
Falling edge
      |
      v
GPIO event
      |
      v
button_irq.c
      |
      v
"BUTTON PRESSED"
```

---

# 16. Pull-Up Configuration

A button input can use an internal pull-up.

Conceptually:

```text
       3.3V
        |
      Pull-up
        |
        +-------- GPIO
        |
      Button
        |
       GND
```

When button is released:

```text
GPIO = 1
```

When button is pressed:

```text
GPIO = 0
```

The libgpiod backend can request the appropriate bias configuration
when supported by the kernel and GPIO controller.

---

# 17. gpio-libgpiod.c Responsibilities

The file:

```text
application/src/gpio-libgpiod.c
```

is the hardware abstraction backend for libgpiod.

Its responsibilities include:

1. Open GPIO chip
2. Find GPIO line
3. Validate GPIO configuration
4. Request GPIO line
5. Configure input/output
6. Configure edge detection
7. Configure bias
8. Read GPIO
9. Write GPIO
10. Wait for GPIO events
11. Release GPIO line
12. Close GPIO chip
13. Report errors

---

# 18. Recommended Backend API

The project can expose a simple API through:

```text
application/include/gpio_libgpiod.h
```

Example:

```c
#ifndef GPIO_LIBGPIOD_H
#define GPIO_LIBGPIOD_H

int gpio_libgpiod_init(int chip_num,
                       unsigned int line,
                       int direction,
                       int initial_value);

int gpio_libgpiod_write(int value);

int gpio_libgpiod_read(void);

int gpio_libgpiod_wait_event(int timeout_ms);

void gpio_libgpiod_cleanup(void);

#endif
```

The common layer can hide the backend implementation from the
application.

---

# 19. Unified Application API

Application examples should preferably not directly depend on
libgpiod internals.

For example:

```c
gpio_config_t config;

gpio_init(&config);

gpio_write(&config, 1);

gpio_write(&config, 0);

gpio_read(&config);

gpio_cleanup(&config);
```

Architecture:

```text
Application
     |
     v
gpio-common.c
     |
     +------------+
     |            |
     v            v
 Sysfs        libgpiod
 backend       backend
```

This makes the project easier to maintain.

---

# 20. GPIO Toggle Example

`gpio_toggle.c`:

```text
Initialize GPIO
      |
      v
Configure OUTPUT
      |
      v
Read current value
      |
      v
Invert value
      |
      v
Write new value
      |
      v
Delay
      |
      v
Repeat
```

Example:

```text
GPIO = 0
   |
   v
GPIO = 1
   |
   v
GPIO = 0
   |
   v
GPIO = 1
```

This is useful for validating the GPIO output path.

---

# 21. LED Blink Example

`led_blink.c` uses the same GPIO backend.

Flow:

```text
GPIO INIT
   |
   v
OUTPUT LOW
   |
   v
GPIO HIGH
   |
  500 ms
   |
   v
GPIO LOW
   |
  500 ms
   |
   v
Repeat
```

This provides a simple board-level GPIO validation test.

---

# 22. Button Example

`button.c` configures a GPIO as input.

Flow:

```text
GPIO INIT
   |
   v
Configure INPUT
   |
   v
Read GPIO
   |
   +---- 0 ---> Button Pressed
   |
   +---- 1 ---> Button Released
   |
   v
Repeat
```

---

# 23. Button IRQ Example

`button_irq.c` uses edge-triggered GPIO events.

Flow:

```text
GPIO INIT
   |
   v
Configure INPUT
   |
   v
Configure EDGE
   |
   v
Wait for event
   |
   +---- Rising Edge
   |
   +---- Falling Edge
   |
   v
Process event
   |
   v
Wait again
```

This avoids continuously polling the GPIO.

---

# 24. Device Tree Relationship

The GPIO controller is normally described in the STM32MP157 device
tree.

Conceptually:

```text
Device Tree
    |
    v
STM32 GPIO Controller
    |
    v
GPIO Bank
    |
    v
GPIO Pin
```

Example GPIO consumer:

```dts
gpio_test {
        compatible = "stm32,gpio-test";

        test-gpios = <&gpioa 5 GPIO_ACTIVE_HIGH>;
};
```

The exact GPIO bank and pin must correspond to the physical STM32MP157
board connection.

---

# 25. Device Tree and libgpiod

The Device Tree does not normally mean that user space directly
controls the GPIO registers.

Instead:

```text
Device Tree
     |
     v
Linux GPIO Driver
     |
     v
GPIO Controller
     |
     v
/dev/gpiochipX
     |
     v
libgpiod
     |
     v
Application
```

Device Tree describes the hardware.

The Linux GPIO subsystem provides the runtime interface.

`libgpiod` provides the user-space API.

---

# 26. Checking GPIO From Target

After booting the board:

```bash
gpiodetect
```

Then:

```bash
gpioinfo
```

Check GPIO devices:

```bash
ls -l /dev/gpiochip*
```

Check kernel GPIO information:

```bash
cat /sys/kernel/debug/gpio
```

Check kernel messages:

```bash
dmesg | grep -i gpio
```

---

# 27. Manual GPIO Testing

Before testing the application, validate the GPIO subsystem using
the gpiod tools.

For example:

```bash
gpiodetect
```

Then inspect:

```bash
gpioinfo
```

For output testing, use the appropriate `gpioset` syntax for the
installed gpiod version.

For input:

```bash
gpioget
```

The exact command syntax differs between libgpiod 1.x and 2.x, so
always check:

```bash
gpioset --help
gpioget --help
```

on the target.

---

# 28. Application Testing

Build:

```bash
make clean
make
```

Check generated applications:

```bash
ls application/examples/
```

Expected:

```text
gpio_toggle
led_blink
button
button_irq
```

Run LED test:

```bash
./led_blink
```

Run GPIO toggle:

```bash
./gpio_toggle
```

Run button test:

```bash
./button
```

Run interrupt test:

```bash
./button_irq
```

---

# 29. Permissions

The application needs permission to access:

```text
/dev/gpiochipX
```

Check:

```bash
ls -l /dev/gpiochip*
```

Example:

```text
crw-rw---- 1 root gpio ... /dev/gpiochip0
```

If required, add the application user to the GPIO group:

```bash
sudo usermod -aG gpio $USER
```

Then log out and log back in.

---

# 30. Debugging

If the application cannot open the GPIO chip:

```bash
ls /dev/gpiochip*
```

If no GPIO devices exist:

```bash
dmesg | grep -i gpio
```

Check:

```bash
gpioinfo
```

If a line is busy:

```bash
gpioinfo gpiochip0
```

Look for:

```text
[used]
```

or a consumer name.

Possible causes:

* Device Tree consumer already owns GPIO
* Another process requested the GPIO
* GPIO pin is used by another peripheral
* Pin mux is incorrect
* GPIO controller is disabled
* Permission problem

---

# 31. Pin Multiplexing

STM32MP157 GPIO pins can have multiple functions.

For example:

```text
GPIO Pin
   |
   +---- GPIO
   |
   +---- UART
   |
   +---- SPI
   |
   +---- I2C
   |
   +---- PWM
```

Therefore, the GPIO cannot be used if the pin is currently configured
for another peripheral.

Device Tree pinctrl configuration determines the selected function.

Example concept:

```text
pinctrl
   |
   +-- GPIO mode
   |
   +-- UART mode
   |
   +-- SPI mode
```

For GPIO testing, make sure the required pin is configured as GPIO.

---

# 32. Debug Checklist

Use this sequence when GPIO does not work.

```text
Step 1
  |
  +--> Check board power
  |
Step 2
  |
  +--> Check Linux boot
  |
Step 3
  |
  +--> Check /dev/gpiochip*
  |
Step 4
  |
  +--> Run gpiodetect
  |
Step 5
  |
  +--> Run gpioinfo
  |
Step 6
  |
  +--> Verify Device Tree
  |
Step 7
  |
  +--> Verify pinctrl
  |
Step 8
  |
  +--> Verify GPIO ownership
  |
Step 9
  |
  +--> Test with gpiod tools
  |
Step 10
  |
  +--> Test application
```

This separates hardware, kernel, Device Tree, and application problems.

---

# 33. Error Handling

The backend should always check return values.

Bad:

```c
gpiod_line_set_value(line, 1);
```

Better:

```c
if (gpiod_line_set_value(line, 1) < 0) {
        perror("Failed to set GPIO");
        return -1;
}
```

Errors should be reported clearly:

```text
Failed to open GPIO chip
Failed to get GPIO line
Failed to request GPIO line
Failed to configure GPIO
Failed to read GPIO
Failed to write GPIO
Failed to wait for GPIO event
```

This is especially important during board bring-up.

---

# 34. Resource Management

Every successful GPIO operation should have a matching cleanup.

```text
Open chip
    |
    v
Get line
    |
    v
Request line
    |
    v
Use GPIO
    |
    v
Release line
    |
    v
Close chip
```

Do not leave GPIO lines permanently requested unless the application
is intentionally designed as a long-running service.

---

# 35. libgpiod vs Sysfs

| Feature            | Sysfs             | libgpiod         |
| ------------------ | ----------------- | ---------------- |
| Interface          | `/sys/class/gpio` | `/dev/gpiochipX` |
| Status             | Legacy            | Modern           |
| GPIO request       | Basic             | Explicit         |
| Events             | Limited           | Supported        |
| Line ownership     | Weak              | Better           |
| Pull configuration | Limited           | Supported        |
| Application API    | File operations   | C library        |
| Recommended        | No for new code   | Yes              |

The STM32MP157-DK2 project therefore uses libgpiod as the preferred
GPIO interface.

---

# 36. Project Test Strategy

The complete GPIO validation should happen in stages.

### Stage 1 - Kernel

```bash
gpiodetect
gpioinfo
```

Verify GPIO controllers.

### Stage 2 - Manual GPIO

Use `gpioset` / `gpioget` according to the installed gpiod version.

Verify physical GPIO behavior.

### Stage 3 - Application

```bash
./gpio_toggle
```

### Stage 4 - LED

```bash
./led_blink
```

### Stage 5 - Button

```bash
./button
```

### Stage 6 - Interrupt

```bash
./button_irq
```

### Stage 7 - Automated Test

```bash
./test.sh
```

This provides a complete validation chain.

---

# 37. End-to-End Project Flow

The complete STM32MP157-DK2 GPIO project flow is:

```text
                    DEVELOPMENT PC
                          |
                          |
                     Yocto Build
                          |
                          v
                +------------------+
                | Linux Kernel     |
                | Device Tree      |
                | RootFS           |
                +--------+---------+
                         |
                         v
                   SD/eMMC Image
                         |
                         v
                STM32MP157-DK2
                         |
                         v
                     U-Boot
                         |
                         v
                  Linux Kernel
                         |
                         v
                  Device Tree
                         |
                         v
                STM32 GPIO Driver
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
              +----------+----------+
              |          |          |
              v          v          v
        led_blink    button    button_irq
              |
              v
        Physical GPIO
              |
              v
          LED/Button
```

---

# 38. Final Validation

The project is considered successfully implemented when all of the
following work:

```text
[✓] STM32MP157-DK2 boots Linux

[✓] GPIO controller detected

[✓] /dev/gpiochipX available

[✓] gpiodetect works

[✓] gpioinfo works

[✓] GPIO line can be requested

[✓] GPIO output works

[✓] LED blink works

[✓] GPIO input works

[✓] Button detection works

[✓] Rising/falling edge events work

[✓] libgpiod backend works

[✓] Common GPIO API works

[✓] Sysfs fallback works where supported

[✓] Device Tree configuration verified

[✓] Automated test passes
```

---

# 39. Important Note About Sysfs

GPIO Sysfs is a legacy interface and is deprecated in modern Linux
kernels.

Therefore, this project should treat:

```text
libgpiod
```

as the primary production interface.

The Sysfs implementation is retained mainly for:

* Legacy Linux systems
* Learning
* Comparison
* Debugging
* Compatibility testing

The preferred architecture is:

```text
Application
     |
     v
Common GPIO API
     |
     v
libgpiod
     |
     v
/dev/gpiochipX
```

---

# 40. Summary

The STM32MP157-DK2 GPIO application provides a complete Linux GPIO
framework demonstration.

The implementation covers:

* Linux GPIO subsystem
* Device Tree GPIO configuration
* STM32 GPIO controller
* GPIO character device
* libgpiod
* GPIO input
* GPIO output
* LED control
* Button monitoring
* GPIO interrupts
* Edge detection
* Pull-up/pull-down
* Sysfs compatibility
* Unified GPIO API
* Automated testing

The key software path is:

```text
User Application
       ↓
gpio-common.c
       ↓
gpio-libgpiod.c
       ↓
libgpiod
       ↓
/dev/gpiochipX
       ↓
Linux GPIO Framework
       ↓
STM32 GPIO Driver
       ↓
STM32MP157 GPIO Controller
       ↓
Physical GPIO Pin
```

This architecture makes the STM32MP157-DK2 project suitable for
embedded Linux BSP development, GPIO driver integration, Device Tree
configuration, and interview-level demonstration of Linux GPIO
subsystem concepts.

```
```

