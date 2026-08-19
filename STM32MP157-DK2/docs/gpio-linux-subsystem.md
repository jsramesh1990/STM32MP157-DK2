# `STM32MP157-DK2/docs/gpio-linux-subsystem.md`

````markdown
# Linux GPIO Subsystem - STM32MP157-DK2

## 1. Overview

The STM32MP157-DK2 project uses the Linux GPIO subsystem to provide
a standard interface between user applications and GPIO hardware.

The GPIO control path is:

Application
    ↓
GPIO Common API
    ↓
libgpiod / Sysfs
    ↓
Linux GPIO Subsystem
    ↓
GPIO Controller Driver
    ↓
Device Tree
    ↓
STM32 GPIO Hardware
    ↓
Physical GPIO Pin

The project supports:

- GPIO output
- GPIO input
- LED control
- Button input
- GPIO interrupt / edge detection
- GPIO value read/write
- GPIO direction configuration
- libgpiod interface
- Legacy sysfs interface
- Device Tree configuration
- GPIO testing from user space

---

# 2. STM32MP157 GPIO Architecture

The STM32MP157 contains multiple GPIO controller banks.

Typical GPIO banks include:

- GPIOA
- GPIOB
- GPIOC
- GPIOD
- GPIOE
- GPIOF
- GPIOG
- GPIOH
- GPIOI
- GPIOJ
- GPIOK

Each GPIO controller contains multiple GPIO lines.

For example:

```text
STM32MP157
    |
    +---------------- GPIOA
    |                   |
    |                   +-- PA0
    |                   +-- PA1
    |                   +-- PA2
    |                   +-- ...
    |
    +---------------- GPIOB
    |                   |
    |                   +-- PB0
    |                   +-- PB1
    |                   +-- ...
    |
    +---------------- GPIOC
    |
    +---------------- GPIOD
    |
    +---------------- GPIOE
    |
    +---------------- GPIOF
    |
    +---------------- GPIOG
    |
    +---------------- GPIOH
    |
    +---------------- GPIOI
    |
    +---------------- GPIOJ
    |
    +---------------- GPIOK
````

Linux does not normally expose these physical GPIO names directly to
applications.

Instead, GPIO controllers are exposed through the Linux GPIO subsystem.

---

# 3. Linux GPIO Subsystem

The Linux GPIO subsystem provides a common framework for GPIO controllers.

The architecture is:

```text
                 User Application
                       |
                       |
                GPIO Application
                       |
             +---------+---------+
             |                   |
          libgpiod              Sysfs
             |                   |
             +---------+---------+
                       |
                       v
                Linux GPIO Core
                       |
                       v
              GPIO Controller API
                       |
                       v
            STM32 GPIO Controller
                       |
                       v
                  GPIO Hardware
```

The Linux GPIO subsystem hides hardware-specific details from the
application.

The application does not need to directly access STM32 GPIO registers.

---

# 4. GPIO Controller Driver

The STM32 GPIO controller is managed by the Linux GPIO driver.

The driver is responsible for:

* GPIO configuration
* GPIO direction
* GPIO input
* GPIO output
* GPIO interrupt handling
* GPIO pin state
* GPIO controller registration
* GPIO Device Tree parsing

The driver registers the GPIO controller with the Linux GPIO subsystem.

Conceptually:

```text
STM32 GPIO Driver
       |
       | gpiochip registration
       v
Linux GPIO Core
       |
       v
/dev/gpiochip0
/dev/gpiochip1
/dev/gpiochip2
...
```

The exact number and numbering of GPIO chips depends on the kernel,
Device Tree configuration and board configuration.

---

# 5. Device Tree Relationship

GPIO hardware is described in the Device Tree.

Example:

```dts
gpioa: gpio@50002000 {
    compatible = "st,stm32-gpio";
    reg = <0x50002000 0x400>;
    gpio-controller;
    #gpio-cells = <2>;
    interrupt-controller;
    #interrupt-cells = <2>;
};
```

The actual address and properties must match the STM32MP157
Device Tree supplied by the BSP/kernel version.

GPIO consumer devices can reference GPIOs using:

```dts
led-gpios = <&gpioa 5 GPIO_ACTIVE_HIGH>;
```

This means:

```text
gpioa
  |
  +-- GPIO line 5
          |
          +-- LED
```

---

# 6. GPIO Consumer Model

A GPIO is normally consumed by another hardware function.

Examples:

```text
GPIO
 |
 +-- LED
 |
 +-- Button
 |
 +-- Relay
 |
 +-- Reset signal
 |
 +-- Enable signal
 |
 +-- Chip select
 |
 +-- Interrupt source
```

In the project, LEDs and buttons are used as GPIO consumers.

Example:

```text
Device Tree
    |
    +-- LED GPIO
    |
    +-- Button GPIO
            |
            v
       Linux GPIO Core
            |
            v
       GPIO Controller
            |
            v
        STM32MP157
```

---

# 7. GPIO Numbering

There are two important concepts:

### Physical GPIO

Example:

```text
PA5
PB3
PC7
```

### Linux GPIO line

Example:

```text
gpiochip0 line 5
gpiochip1 line 3
```

These are not necessarily the same number.

Therefore, the application should not blindly assume:

```text
GPIO17 = physical GPIO17
```

Instead, inspect the GPIO controller.

Use:

```bash
gpiodetect
```

Example:

```text
gpiochip0 [gpioa] (16 lines)
gpiochip1 [gpiob] (16 lines)
gpiochip2 [gpioc] (16 lines)
```

Then inspect:

```bash
gpioinfo
```

Example:

```text
gpiochip0 - 16 lines:
        line   0: "PA0"
        line   1: "PA1"
        line   2: "PA2"
        line   3: "PA3"
        ...
```

The exact output depends on the BSP.

---

# 8. libgpiod Interface

Modern Linux systems use the GPIO character-device interface.

GPIO chips are exposed as:

```text
/dev/gpiochip0
/dev/gpiochip1
/dev/gpiochip2
...
```

The application communicates with these devices through libgpiod.

Project flow:

```text
gpio_toggle.c
      |
      v
gpio_common.h
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
Linux GPIO Core
      |
      v
STM32 GPIO Driver
      |
      v
STM32 GPIO Hardware
```

---

# 9. libgpiod Basic Operations

First identify GPIO chips:

```bash
gpiodetect
```

Then inspect lines:

```bash
gpioinfo
```

Set a GPIO output:

```bash
gpioset gpiochip0 5=1
```

Set it low:

```bash
gpioset gpiochip0 5=0
```

Read an input:

```bash
gpioget gpiochip0 6
```

Monitor GPIO events:

```bash
gpiomon gpiochip0 6
```

These commands are useful for board-level debugging.

---

# 10. Project libgpiod Implementation

The project contains:

```text
application/
├── include/
│   ├── gpio_common.h
│   └── gpio_libgpiod.h
│
└── src/
    ├── gpio-common.c
    └── gpio-libgpiod.c
```

The implementation separates the application from the GPIO backend.

Example:

```text
Application
     |
     v
gpio_init()
     |
     v
gpio_write()
     |
     v
gpio_read()
     |
     v
gpio_cleanup()
     |
     v
libgpiod backend
```

This makes the application independent of the underlying GPIO API.

---

# 11. GPIO Output Flow

For an LED:

```text
Application
    |
    | gpio_write(GPIO, 1)
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
Linux GPIO Core
    |
    v
STM32 GPIO Driver
    |
    v
GPIO Controller
    |
    v
Physical Pin
    |
    v
LED ON
```

For LOW:

```text
Application
    |
    | gpio_write(GPIO, 0)
    v
libgpiod
    |
    v
Linux GPIO
    |
    v
STM32 GPIO Controller
    |
    v
Physical Pin LOW
    |
    v
LED OFF
```

---

# 12. GPIO Input Flow

For a push button:

```text
Physical Button
      |
      v
STM32 GPIO Pin
      |
      v
STM32 GPIO Driver
      |
      v
Linux GPIO Core
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
button.c
```

Application:

```c
value = gpio_read(&config);
```

If:

```text
value = 0
```

the button may be considered pressed depending on the electrical
configuration.

---

# 13. GPIO Interrupt Flow

GPIO interrupts are important for event-driven button handling.

The flow is:

```text
Button Press
     |
     v
Physical GPIO Transition
     |
     v
STM32 GPIO Interrupt
     |
     v
GPIO Controller Driver
     |
     v
Linux IRQ Subsystem
     |
     v
GPIO Character Device
     |
     v
libgpiod Event
     |
     v
button_irq.c
     |
     v
Application Callback/Event
```

For example:

```text
Falling Edge
     |
     v
GPIO interrupt
     |
     v
libgpiod event
     |
     v
Application
     |
     +-- "BUTTON PRESSED"
```

---

# 14. Edge Detection

The project supports:

```text
none
rising
falling
both
```

### Rising Edge

```text
0 ───────────────┐
                 │
                 └──────── 1
                      ^
                      |
                 Rising edge
```

### Falling Edge

```text
1 ───────────────┐
                 │
                 └──────── 0
                      ^
                      |
                 Falling edge
```

### Both Edges

```text
0 → 1
1 → 0
```

Both transitions generate events.

---

# 15. Button Interrupt Example

The project contains:

```text
application/examples/button_irq.c
```

Conceptual flow:

```text
Initialize GPIO
      |
      v
Configure INPUT
      |
      v
Configure Pull-up
      |
      v
Configure Falling Edge
      |
      v
Wait for GPIO Event
      |
      v
Button Press
      |
      v
Interrupt/Event
      |
      v
Application handles event
```

This avoids continuously polling the GPIO.

---

# 16. GPIO Polling vs Interrupt

## Polling

```text
while (1)
{
    read_gpio();

    sleep();
}
```

Flow:

```text
CPU
 |
 +-- Read GPIO
 |
 +-- Sleep
 |
 +-- Read GPIO
 |
 +-- Sleep
```

Advantages:

* Simple
* Easy to understand

Disadvantages:

* CPU overhead
* Response depends on polling interval
* Less efficient

---

## Interrupt/Event Driven

```text
wait_for_gpio_event();
```

Flow:

```text
CPU
 |
 | sleeping
 |
 | GPIO event
 v
Wake up
 |
 v
Process event
```

Advantages:

* Efficient
* Better event response
* No continuous polling

For button applications, interrupt/event-driven operation is preferred.

---

# 17. Sysfs Interface

The project also contains:

```text
gpio-sysfs.c
gpio_sysfs.h
```

Sysfs GPIO is a legacy interface.

Typical paths:

```text
/sys/class/gpio/
```

Traditional operations:

```bash
echo 17 > /sys/class/gpio/export
echo out > /sys/class/gpio/gpio17/direction
echo 1 > /sys/class/gpio/gpio17/value
cat /sys/class/gpio/gpio17/value
echo 17 > /sys/class/gpio/unexport
```

However, GPIO sysfs has been deprecated in modern Linux kernels.

For new designs, libgpiod is preferred.

---

# 18. Why Keep Sysfs in This Project?

The project intentionally supports both:

```text
                 GPIO Common API
                       |
             +---------+---------+
             |                   |
             v                   v
         libgpiod              Sysfs
          Modern               Legacy
```

This provides:

* Backward compatibility
* Learning value
* Comparison between APIs
* Debugging support
* Support for older BSPs

The preferred implementation for STM32MP157 Linux is:

```text
libgpiod
```

---

# 19. GPIO Common Layer

The common layer is:

```text
application/src/gpio-common.c
```

Its purpose is to hide backend-specific implementation.

The application calls:

```c
gpio_init();
gpio_write();
gpio_read();
gpio_cleanup();
```

It does not directly care whether the implementation uses:

```text
libgpiod
```

or:

```text
sysfs
```

Architecture:

```text
              Application
                   |
                   v
           GPIO Common API
                   |
        +----------+----------+
        |                     |
        v                     v
   libgpiod backend      sysfs backend
        |                     |
        v                     v
 Linux GPIO subsystem   /sys/class/gpio
```

---

# 20. GPIO Configuration

GPIO configuration can be stored in:

```text
configs/gpio-test-config.json
```

Example:

```json
{
    "led": {
        "chip": 0,
        "line": 5,
        "direction": "output",
        "active_low": false
    },
    "button": {
        "chip": 0,
        "line": 6,
        "direction": "input",
        "edge": "falling",
        "bias": "pull_up"
    }
}
```

The actual GPIO chip and line values must be verified on the target
STM32MP157-DK2 board.

---

# 21. GPIO Direction

GPIO direction determines whether the processor drives the pin or
reads the pin.

## Output

```text
CPU
 |
 v
GPIO Controller
 |
 v
Physical Pin
```

Example:

```c
gpio_write(&config, 1);
```

## Input

```text
Physical Pin
 |
 v
GPIO Controller
 |
 v
CPU
```

Example:

```c
value = gpio_read(&config);
```

---

# 22. Pull-up and Pull-down

Input GPIOs can be electrically configured using pull resistors.

### Pull-up

```text
VCC
 |
[Pull-up]
 |
GPIO -------- Button -------- GND
```

Default state:

```text
GPIO = 1
```

Button pressed:

```text
GPIO = 0
```

### Pull-down

```text
GPIO -------- Button -------- VCC
 |
[Pull-down]
 |
GND
```

Default state:

```text
GPIO = 0
```

Button pressed:

```text
GPIO = 1
```

The Device Tree pinctrl configuration and GPIO line configuration must
match the board hardware.

---

# 23. Pin Multiplexing

STM32MP157 pins can have multiple alternate functions.

For example, a physical pin may be configured as:

```text
GPIO
UART
SPI
I2C
PWM
```

Therefore, GPIO functionality depends on pin multiplexing.

Conceptually:

```text
STM32 Pin
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

The Device Tree pinctrl configuration selects the required function.

Example:

```dts
&pinctrl {
    gpio_test_pins: gpio-test-pins {
        pins {
            pinmux = <STM32_PINMUX('A', 5, GPIO)>;
            bias-pull-up;
            drive-push-pull;
            slew-rate = <0>;
        };
    };
};
```

The exact pinmux syntax must follow the STM32MP157 kernel Device Tree
binding used by the selected BSP/kernel version.

---

# 24. Device Tree → GPIO Driver Flow

During Linux boot:

```text
Bootloader
    |
    v
Linux Kernel
    |
    v
Device Tree loaded
    |
    v
STM32 GPIO node detected
    |
    v
GPIO driver probe()
    |
    v
GPIO controller registered
    |
    v
/dev/gpiochipX created
```

After boot:

```bash
ls /dev/gpiochip*
```

Example:

```text
/dev/gpiochip0
/dev/gpiochip1
/dev/gpiochip2
```

---

# 25. Driver Probe Flow

Conceptually:

```text
Kernel starts
     |
     v
Device Tree parsing
     |
     v
Compatible string matched
     |
     v
STM32 GPIO driver
     |
     v
probe()
     |
     +-- Map registers
     |
     +-- Configure GPIO controller
     |
     +-- Configure interrupts
     |
     +-- Register gpio_chip
     |
     v
Linux GPIO subsystem
```

After successful registration, applications can access the GPIO
controller through the GPIO character-device interface.

---

# 26. GPIO Character Device

Modern Linux GPIO uses character devices.

Example:

```text
/dev/gpiochip0
```

The application opens:

```text
/dev/gpiochip0
```

Then requests a GPIO line.

Conceptually:

```text
open()
   |
   v
/dev/gpiochip0
   |
   v
GPIO chip
   |
   v
Request line
   |
   v
Configure direction
   |
   v
Read/write/event
   |
   v
Release line
```

---

# 27. GPIO Resource Ownership

Only one consumer should normally control a GPIO line at a time.

Example:

```text
Application A
     |
     +---- GPIO5
```

Application B should not simultaneously request:

```text
GPIO5
```

This avoids:

```text
Device or resource busy
```

errors.

Check ownership with:

```bash
gpioinfo
```

---

# 28. GPIO Testing on STM32MP157-DK2

After booting the board, first check:

```bash
uname -a
```

Then:

```bash
ls /dev/gpiochip*
```

Detect chips:

```bash
gpiodetect
```

Inspect lines:

```bash
gpioinfo
```

Then test an output GPIO:

```bash
gpioset gpiochip0 <line>=1
```

Read an input GPIO:

```bash
gpioget gpiochip0 <line>
```

Monitor events:

```bash
gpiomon gpiochip0 <line>
```

Replace `<line>` with a GPIO line that is actually available on the
board.

---

# 29. Project Application Flow

The project examples are:

```text
application/
├── examples/
│   ├── gpio_toggle.c
│   ├── led_blink.c
│   ├── button.c
│   └── button_irq.c
│
├── include/
│   ├── gpio_common.h
│   ├── gpio_libgpiod.h
│   └── gpio_sysfs.h
│
└── src/
    ├── gpio-common.c
    ├── gpio-libgpiod.c
    └── gpio-sysfs.c
```

Application flow:

```text
                 +----------------+
                 | gpio_toggle.c  |
                 +-------+--------+
                         |
                 +-------v--------+
                 | GPIO Common    |
                 +-------+--------+
                         |
                 +-------v--------+
                 | libgpiod       |
                 +-------+--------+
                         |
                 +-------v--------+
                 | Linux GPIO     |
                 +-------+--------+
                         |
                 +-------v--------+
                 | STM32 Driver   |
                 +-------+--------+
                         |
                 +-------v--------+
                 | STM32MP157     |
                 +----------------+
```

---

# 30. LED Blink Flow

`led_blink.c` performs:

```text
Start
 |
 v
Open GPIO chip
 |
 v
Request GPIO line
 |
 v
Configure OUTPUT
 |
 v
Set GPIO HIGH
 |
 v
Delay
 |
 v
Set GPIO LOW
 |
 v
Delay
 |
 v
Repeat
 |
 v
Release GPIO
 |
 v
Exit
```

---

# 31. Button Flow

`button.c` performs:

```text
Start
 |
 v
Open GPIO chip
 |
 v
Request GPIO line
 |
 v
Configure INPUT
 |
 v
Read GPIO
 |
 v
Display button state
 |
 v
Repeat
```

---

# 32. Button Interrupt Flow

`button_irq.c` performs:

```text
Start
 |
 v
Open GPIO chip
 |
 v
Request GPIO input
 |
 v
Configure edge detection
 |
 v
Wait for event
 |
 v
Button changes state
 |
 v
GPIO event generated
 |
 v
Application wakes
 |
 v
Process event
 |
 v
Wait for next event
```

This is the recommended approach for event-driven GPIO applications.

---

# 33. GPIO Debugging

Check GPIO controllers:

```bash
gpiodetect
```

Check GPIO lines:

```bash
gpioinfo
```

Check kernel GPIO state:

```bash
cat /sys/kernel/debug/gpio
```

Check kernel messages:

```bash
dmesg | grep -i gpio
```

Check Device Tree:

```bash
ls /proc/device-tree/
```

Check GPIO-related Device Tree:

```bash
find /proc/device-tree -iname "*gpio*"
```

Check pinctrl:

```bash
cat /sys/kernel/debug/pinctrl/*/pinmux-pins
```

Check GPIO controllers:

```bash
ls /sys/class/gpio/
```

---

# 34. Common Debugging Problems

## Problem 1: `/dev/gpiochip0` does not exist

Check:

```bash
ls /dev/gpiochip*
```

If missing, check:

```bash
dmesg | grep -i gpio
```

Possible causes:

* GPIO driver not enabled
* Device Tree node disabled
* Kernel configuration missing
* Driver probe failure

---

## Problem 2: GPIO line is busy

Check:

```bash
gpioinfo
```

The line may already be owned by:

* LED driver
* Button driver
* SPI
* I2C
* UART
* PWM
* Another application

---

## Problem 3: GPIO output does not change

Check:

```bash
gpioinfo
```

Then verify:

```text
Pinmux
GPIO direction
GPIO line
Device Tree
Hardware connection
```

Also inspect:

```bash
cat /sys/kernel/debug/pinctrl/*/pinmux-pins
```

---

## Problem 4: Button always reads the same value

Check:

```text
Pull-up/pull-down
Button wiring
GPIO direction
Pinmux
GPIO active-low configuration
```

---

## Problem 5: Interrupt is not generated

Check:

```text
GPIO input configuration
Edge configuration
IRQ mapping
Device Tree
Pinmux
Button electrical connection
```

Monitor using:

```bash
gpiomon gpiochip0 <line>
```

---

# 35. Kernel Configuration

The GPIO subsystem requires appropriate kernel configuration.

A typical configuration includes:

```text
CONFIG_GPIOLIB=y
CONFIG_GPIO_CDEV=y
```

Depending on kernel version and BSP:

```text
CONFIG_GPIO_SYSFS=y
```

may be required for legacy sysfs GPIO support.

The project configuration file is:

```text
configs/stm32mp157_gpio_defconfig
```

The exact required options should be verified against the STM32MP157
kernel version being used.

Check the running kernel:

```bash
zcat /proc/config.gz | grep GPIO
```

if `/proc/config.gz` is enabled.

Alternatively:

```bash
grep GPIO /boot/config-$(uname -r)
```

---

# 36. Build Integration

The GPIO application is built separately from the kernel GPIO driver.

The relationship is:

```text
                 Yocto Build
                     |
        +------------+------------+
        |                         |
        v                         v
 Linux Kernel                Application
        |                         |
        v                         v
GPIO Driver                 GPIO Program
        |                         |
        +------------+------------+
                     |
                     v
                RootFS Image
                     |
                     v
              STM32MP157-DK2
```

The kernel provides:

```text
GPIO subsystem
GPIO controller driver
Device Tree
GPIO character device
```

The application provides:

```text
gpio_toggle
led_blink
button
button_irq
```

---

# 37. Yocto Integration

The application can be packaged using a custom Yocto recipe.

Example structure:

```text
meta-stm32mp157-gpio/
|
+-- conf/
|   +-- layer.conf
|
+-- recipes-gpio/
    +-- gpio-control/
        |
        +-- gpio-control.bb
        |
        +-- files/
            |
            +-- gpio_toggle.c
            +-- led_blink.c
            +-- button.c
            +-- button_irq.c
            +-- gpio-common.c
            +-- gpio-libgpiod.c
            +-- gpio_common.h
            +-- gpio_libgpiod.h
            +-- Makefile
```

The recipe installs the GPIO applications into the target RootFS.

---

# 38. Runtime Architecture

After boot:

```text
+--------------------------------------------------+
|                 STM32MP157 Linux                 |
|                                                  |
|  +--------------------------------------------+  |
|  | User Space                                 |  |
|  |                                            |  |
|  | led_blink                                  |  |
|  | gpio_toggle                                |  |
|  | button                                     |  |
|  | button_irq                                 |  |
|  +----------------------+---------------------+  |
|                         |                        |
|                  libgpiod API                    |
|                         |                        |
|  +----------------------v---------------------+  |
|  | Linux GPIO Character Device                |  |
|  | /dev/gpiochipX                            |  |
|  +----------------------+---------------------+  |
|                         |                        |
|  +----------------------v---------------------+  |
|  | Linux GPIO Subsystem                      |  |
|  +----------------------+---------------------+  |
|                         |                        |
|  +----------------------v---------------------+  |
|  | STM32 GPIO Driver                         |  |
|  +----------------------+---------------------+  |
|                         |                        |
|  +----------------------v---------------------+  |
|  | STM32MP157 GPIO Hardware                  |  |
|  +--------------------------------------------+  |
+--------------------------------------------------+
```

---

# 39. Complete Project Flow

The complete STM32MP157-DK2 GPIO project works as follows:

```text
                 Development PC
                       |
                       |
                 Yocto Build
                       |
             +---------+---------+
             |                   |
             v                   v
        Linux Kernel        GPIO Application
             |                   |
             |                   |
        Device Tree          C Programs
             |                   |
             +---------+---------+
                       |
                       v
                  RootFS/Image
                       |
                       v
                 SD Card / eMMC
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
              GPIO Driver Probe
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
               GPIO Application
                       |
             +---------+---------+
             |                   |
             v                   v
          LED GPIO           Button GPIO
             |                   |
             v                   v
        GPIO Output        GPIO Input/Event
```

---

# 40. Recommended Development Sequence

For this STM32MP157-DK2 project, development should follow this order.

### Step 1 - Board Bring-up

Verify:

```bash
uname -a
cat /proc/cpuinfo
```

### Step 2 - Check GPIO subsystem

```bash
gpiodetect
gpioinfo
```

### Step 3 - Verify pinmux

```bash
cat /sys/kernel/debug/pinctrl/*/pinmux-pins
```

### Step 4 - Test GPIO manually

```bash
gpioset gpiochipX LINE=1
gpioset gpiochipX LINE=0
```

### Step 5 - Develop common GPIO API

Implement:

```text
gpio-common.c
gpio_common.h
```

### Step 6 - Implement libgpiod backend

Implement:

```text
gpio-libgpiod.c
gpio_libgpiod.h
```

### Step 7 - Develop LED application

```text
led_blink.c
gpio_toggle.c
```

### Step 8 - Develop button application

```text
button.c
```

### Step 9 - Add interrupt support

```text
button_irq.c
```

### Step 10 - Add Device Tree configuration

```text
stm32mp157-gpio-test.dts
stm32mp157-gpio-test-overlay.dts
```

### Step 11 - Add Yocto integration

Build applications into RootFS.

### Step 12 - Hardware validation

Test:

```text
LED
Button
GPIO read/write
GPIO interrupts
Multiple GPIOs
Error handling
```

---

# 41. Validation Matrix

| Test             | Method          | Expected Result                   |
| ---------------- | --------------- | --------------------------------- |
| GPIO detection   | `gpiodetect`    | GPIO chips detected               |
| GPIO information | `gpioinfo`      | Lines visible                     |
| Output HIGH      | `gpioset`       | Pin becomes HIGH                  |
| Output LOW       | `gpioset`       | Pin becomes LOW                   |
| Input read       | `gpioget`       | Correct state                     |
| Rising event     | `gpiomon`       | Rising event received             |
| Falling event    | `gpiomon`       | Falling event received            |
| LED blink        | `led_blink`     | LED toggles                       |
| GPIO toggle      | `gpio_toggle`   | GPIO changes state                |
| Button polling   | `button`        | Button state displayed            |
| Button IRQ       | `button_irq`    | Event generated                   |
| Sysfs            | `gpio-sysfs`    | Legacy interface works if enabled |
| libgpiod         | `gpio-libgpiod` | Modern interface works            |

---

# 42. Important Design Principle

The application should **not directly manipulate STM32 GPIO
registers**.

Avoid:

```text
Application
     |
     v
STM32 GPIO registers
```

Use:

```text
Application
     |
     v
libgpiod
     |
     v
Linux GPIO subsystem
     |
     v
STM32 GPIO driver
     |
     v
Hardware
```

This provides:

* Hardware abstraction
* Kernel ownership
* Standard Linux APIs
* Interrupt support
* Better portability
* Better maintainability
* Proper resource management

---

# 43. Final Architecture

The final project architecture is:

```text
                    STM32MP157-DK2
                           |
                    +------+------+
                    |             |
                 LED GPIO      Button GPIO
                    |             |
                    +------+------+
                           |
                    GPIO Hardware
                           |
                    STM32 GPIO Driver
                           |
                    Linux GPIO Core
                           |
              +------------+------------+
              |                         |
       /dev/gpiochipX              Sysfs GPIO
              |                    (Legacy)
              |
           libgpiod
              |
       gpio-libgpiod.c
              |
       gpio-common.c
              |
     +--------+--------+
     |        |        |
     v        v        v
led_blink  gpio_toggle button
                         |
                         v
                    button_irq
```

## 44. Key Takeaway

For the **STM32MP157-DK2 GPIO Control & Simulator project**, the
recommended production architecture is:

```text
Device Tree
     ↓
STM32 GPIO Driver
     ↓
Linux GPIO Subsystem
     ↓
/dev/gpiochipX
     ↓
libgpiod
     ↓
GPIO Common API
     ↓
Application
     ↓
LED / Button / GPIO Hardware
```

The **sysfs implementation should be treated as a legacy compatibility
backend**, while **libgpiod should be the primary GPIO interface** for
the STM32MP157 Linux application.

```
```

