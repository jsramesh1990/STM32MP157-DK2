Create this file:

`STM32MP157-DK2/docs/device-tree.md`

````markdown
# STM32MP157-DK2 Device Tree Guide

## Table of Contents

- [1. Overview](#1-overview)
- [2. Why Device Tree Is Required](#2-why-device-tree-is-required)
- [3. Device Tree Architecture](#3-device-tree-architecture)
- [4. Project Device Tree Files](#4-project-device-tree-files)
- [5. STM32MP157 GPIO Architecture](#5-stm32mp157-gpio-architecture)
- [6. GPIO Device Tree Configuration](#6-gpio-device-tree-configuration)
- [7. LED GPIO Configuration](#7-led-gpio-configuration)
- [8. Button GPIO Configuration](#8-button-gpio-configuration)
- [9. GPIO Interrupt Configuration](#9-gpio-interrupt-configuration)
- [10. GPIO Controller](#10-gpio-controller)
- [11. Pin Controller and Pinmux](#11-pin-controller-and-pinmux)
- [12. Device Tree Overlay](#12-device-tree-overlay)
- [13. Device Tree Compilation](#13-device-tree-compilation)
- [14. Deploying the DTB](#14-deploying-the-dtb)
- [15. U-Boot Device Tree Flow](#15-u-boot-device-tree-flow)
- [16. Linux Device Tree Parsing](#16-linux-device-tree-parsing)
- [17. GPIO Driver Probe Flow](#17-gpio-driver-probe-flow)
- [18. Verifying Device Tree at Runtime](#18-verifying-device-tree-at-runtime)
- [19. GPIO Testing](#19-gpio-testing)
- [20. Troubleshooting](#20-troubleshooting)
- [21. Complete Device Tree Flow](#21-complete-device-tree-flow)

---

# 1. Overview

The **STM32MP157-DK2 GPIO Control & Simulator System** uses the Linux
Device Tree to describe the GPIO hardware configuration of the
STM32MP157 processor.

The Device Tree provides hardware information to the Linux kernel.

For this project, the Device Tree is mainly responsible for describing:

- GPIO controllers
- GPIO pins
- Pin multiplexing
- LED GPIO
- Button GPIO
- GPIO interrupts
- Pull-up/pull-down configuration
- GPIO polarity
- Peripheral ownership

The basic relationship is:

```text
Device Tree
     |
     v
Linux Kernel
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
Application
````

---

# 2. Why Device Tree Is Required

Linux needs to know how the physical hardware is connected.

For example, the application may say:

```text
Use GPIO line 5
```

But Linux also needs to know:

```text
Which GPIO controller?
Which physical pin?
Which alternate function?
Input or output?
Pull-up or pull-down?
Active high or active low?
Is it an interrupt?
```

Device Tree provides this hardware description.

Without correct Device Tree configuration:

```text
Application
     |
     v
libgpiod
     |
     v
/dev/gpiochipX
     |
     X
Incorrect GPIO configuration
```

With correct configuration:

```text
Application
     |
     v
libgpiod
     |
     v
GPIO character device
     |
     v
Linux GPIO subsystem
     |
     v
STM32 GPIO driver
     |
     v
Correct GPIO pin
```

---

# 3. Device Tree Architecture

The complete Device Tree architecture is:

```text
                     Device Tree Source
                           (.dts)
                              |
                              v
                       Device Tree Compiler
                             (dtc)
                              |
                              v
                       Device Tree Blob
                            (.dtb)
                              |
                              v
                            U-Boot
                              |
                              v
                       Linux Kernel
                              |
                              v
                    Device Tree Framework
                              |
              +---------------+---------------+
              |                               |
              v                               v
        pinctrl driver                  GPIO driver
              |                               |
              v                               v
       Pin Multiplexing                 GPIO Controller
                                              |
                                              v
                                      Linux GPIO Subsystem
```

---

# 4. Project Device Tree Files

The project contains:

```text
STM32MP157-DK2/
|
+-- device-tree/
    |
    +-- README.md
    |
    +-- stm32mp157-gpio-test.dts
    |
    +-- stm32mp157-gpio-test-overlay.dts
```

### `stm32mp157-gpio-test.dts`

Main Device Tree Source file.

It defines the hardware configuration required for the GPIO project.

### `stm32mp157-gpio-test-overlay.dts`

Optional overlay used to modify or extend the base Device Tree.

Typical use cases:

```text
Add GPIO
Add LED
Add Button
Add Interrupt
Modify pinmux
Enable peripheral
Disable peripheral
```

---

# 5. STM32MP157 GPIO Architecture

The STM32MP157 contains multiple GPIO banks.

GPIO ports are typically represented as:

```text
GPIOA
GPIOB
GPIOC
GPIOD
GPIOE
GPIOF
GPIOG
GPIOH
GPIOI
GPIOJ
GPIOK
```

Each GPIO controller contains multiple GPIO lines.

Conceptually:

```text
STM32MP157
     |
     +---------------- GPIOA
     |                    |
     |                    +-- GPIOA0
     |                    +-- GPIOA1
     |                    +-- GPIOA2
     |                    +-- ...
     |
     +---------------- GPIOB
     |                    |
     |                    +-- GPIOB0
     |                    +-- GPIOB1
     |                    +-- ...
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
```

The exact GPIO line available for the project depends on the selected
physical pin and board routing.

---

# 6. GPIO Device Tree Configuration

A GPIO consumer generally references a GPIO controller using a GPIO
specifier.

Conceptually:

```dts
device {
    gpios = <&gpioX Y GPIO_ACTIVE_HIGH>;
};
```

Where:

```text
gpioX
   |
   +-- GPIO controller

Y
   |
   +-- GPIO line/offset

GPIO_ACTIVE_HIGH
   |
   +-- GPIO polarity
```

For an active-low signal:

```dts
gpios = <&gpioX Y GPIO_ACTIVE_LOW>;
```

The exact GPIO controller label and line number must match the
STM32MP157 board's Device Tree and schematic.

---

# 7. LED GPIO Configuration

The project can define an LED as a GPIO-controlled device.

Example:

```dts
led_gpio {
    compatible = "gpio-leds";

    led_test {
        label = "stm32-gpio-test-led";
        gpios = <&gpioX Y GPIO_ACTIVE_HIGH>;
        default-state = "off";
    };
};
```

Conceptual flow:

```text
Device Tree
     |
     v
gpio-leds
     |
     v
GPIO controller
     |
     v
GPIO line
     |
     v
Physical LED
```

When the GPIO is driven high:

```text
GPIO = 1
   |
   v
LED ON
```

When the GPIO is driven low:

```text
GPIO = 0
   |
   v
LED OFF
```

For an active-low LED, the polarity is reversed.

---

# 8. Button GPIO Configuration

A GPIO button can be described as:

```dts
gpio_keys {
    compatible = "gpio-keys";

    button_test {
        label = "gpio-test-button";
        gpios = <&gpioX Y GPIO_ACTIVE_LOW>;
        linux,code = <KEY_ENTER>;
    };
};
```

Conceptual hardware:

```text
       VCC
        |
      Pull-up
        |
        +--------- GPIO
        |
      Button
        |
       GND
```

Therefore:

```text
Button released
      |
      v
GPIO = 1

Button pressed
      |
      v
GPIO = 0
```

---

# 9. GPIO Interrupt Configuration

For interrupt-driven buttons, the Device Tree can describe the
interrupt relationship.

Conceptually:

```dts
button_test {
    compatible = "gpio-keys";
    gpios = <&gpioX Y GPIO_ACTIVE_LOW>;
    interrupt-parent = <&gpioX>;
    interrupts = <Y IRQ_TYPE_EDGE_FALLING>;
};
```

Depending on the GPIO framework and binding used, the exact interrupt
properties may differ.

The interrupt flow is:

```text
Button Press
     |
     v
GPIO Pin
     |
     v
GPIO Controller
     |
     v
Interrupt Controller
     |
     v
Linux IRQ Subsystem
     |
     v
GPIO / Input Driver
     |
     v
User Space Event
```

Supported edge types may include:

```text
IRQ_TYPE_EDGE_RISING
IRQ_TYPE_EDGE_FALLING
IRQ_TYPE_EDGE_BOTH
```

For a pull-up button:

```text
Released = HIGH
Pressed  = LOW
```

Therefore the button press normally generates:

```text
HIGH -> LOW
     |
     v
Falling Edge
```

---

# 10. GPIO Controller

The STM32 GPIO controller is normally already defined in the
STM32MP157 base Device Tree.

Conceptually:

```dts
gpioX: gpio@address {
    compatible = "st,stm32-gpio";
    gpio-controller;
    #gpio-cells = <2>;
    interrupt-controller;
    #interrupt-cells = <2>;
};
```

The actual controller nodes, addresses, clocks, interrupts and
compatible strings should come from the STM32MP157 kernel Device Tree
for the board.

The important properties are:

### `gpio-controller`

Indicates that this node represents a GPIO controller.

```dts
gpio-controller;
```

### `#gpio-cells`

Defines the number of cells used in GPIO specifiers.

```dts
#gpio-cells = <2>;
```

Conceptually:

```text
<&gpioX line flags>
       |    |
       |    +-- GPIO flags
       |
       +------- GPIO line
```

---

# 11. Pin Controller and Pinmux

GPIO configuration is not only about the GPIO controller.

The physical processor pin must also be configured correctly.

This is handled by the STM32 pin controller.

Conceptually:

```text
                 STM32MP157 Pin
                       |
                       v
                 Pin Controller
                       |
             +---------+---------+
             |                   |
             v                   v
          GPIO Mode          Alternate Function
             |
             v
         GPIO Controller
```

For GPIO mode:

```dts
pinctrl_gpio_test: gpio-test {
    pins {
        pinmux = <...>;
        bias-pull-up;
        drive-push-pull;
        slew-rate = <...>;
    };
};
```

The exact `pinmux` values must correspond to the selected STM32MP157
pin.

---

## 11.1 Pin Multiplexing

STM32 pins can normally support multiple functions.

For example, a physical pin may support:

```text
GPIO
UART
SPI
I2C
PWM
Other alternate functions
```

Conceptually:

```text
                 Physical Pin
                      |
       +--------------+--------------+
       |              |              |
       v              v              v
     GPIO           UART           SPI
```

Only one function can normally be selected at a time.

Therefore Device Tree pinctrl configuration is critical.

---

## 11.2 Pull Configuration

Inputs can require pull resistors.

Example:

```dts
bias-pull-up;
```

or:

```dts
bias-pull-down;
```

Conceptually:

```text
GPIO Input
    |
    +---- Pull-up
    |
    +---- Button
```

Without an appropriate pull configuration, an input may float.

---

# 12. Device Tree Overlay

An overlay allows additional hardware configuration without modifying
the complete base Device Tree.

Example:

```text
Base Device Tree
       |
       v
STM32MP157 Board
       |
       v
GPIO Overlay
       |
       v
Additional GPIO configuration
```

Example conceptual overlay:

```dts
/dts-v1/;
/plugin/;

&gpioX {
    gpio_test_pin {
        /* GPIO configuration */
    };
};
```

Another example:

```dts
&some_node {
    status = "okay";
};
```

The overlay can be used to:

```text
Enable GPIO
Enable peripheral
Add LED
Add Button
Change pinctrl
Add interrupt configuration
```

---

# 13. Device Tree Compilation

Device Tree Source files use:

```text
.dts
.dtsi
.dts overlay
```

They are compiled into:

```text
.dtb
```

using the Device Tree Compiler:

```bash
dtc
```

Example:

```bash
dtc -I dts -O dtb \
    -o stm32mp157-gpio-test.dtb \
    stm32mp157-gpio-test.dts
```

However, in a real Linux/Yocto BSP, the Device Tree is normally
compiled by the kernel/Yocto build system rather than manually using
`dtc`.

---

# 14. Deploying the DTB

After compilation:

```text
stm32mp157-gpio-test.dts
             |
             v
          dtc/build
             |
             v
stm32mp157-gpio-test.dtb
             |
             v
          Boot Media
             |
             v
           U-Boot
             |
             v
        Linux Kernel
```

Depending on the boot setup, the DTB may be placed in the boot
partition.

Typical Linux boot files can include:

```text
/boot/
    |
    +-- zImage
    +-- Image
    +-- stm32mp157-*.dtb
    +-- extlinux/
```

The exact boot layout depends on the BSP and boot configuration.

---

# 15. U-Boot Device Tree Flow

U-Boot loads both the kernel and Device Tree.

Conceptually:

```text
SD Card
   |
   +---- Kernel
   |
   +---- DTB
   |
   v
U-Boot
   |
   +---- Kernel -> DDR
   |
   +---- DTB    -> DDR
   |
   v
Boot Linux
```

Linux receives the Device Tree address during boot.

Conceptually:

```text
U-Boot
   |
   | bootargs
   | kernel address
   | DTB address
   v
Linux Kernel
```

---

# 16. Linux Device Tree Parsing

During early kernel initialization:

```text
Linux Kernel
      |
      v
Device Tree Blob
      |
      v
Flattened Device Tree Parser
      |
      v
Hardware Nodes
      |
      +---- GPIO
      |
      +---- pinctrl
      |
      +---- UART
      |
      +---- I2C
      |
      +---- SPI
      |
      +---- USB
```

Linux creates platform devices from the Device Tree.

---

# 17. GPIO Driver Probe Flow

The GPIO driver uses the Device Tree information during probe.

The simplified flow is:

```text
Device Tree
     |
     v
Compatible String
     |
     v
STM32 GPIO Driver
     |
     v
Driver Probe()
     |
     +---- Map registers
     |
     +---- Enable clock
     |
     +---- Configure GPIO
     |
     +---- Register GPIO controller
     |
     v
Linux GPIO Subsystem
```

After successful registration:

```text
/dev/gpiochipX
```

becomes available.

---

# 18. Verifying Device Tree at Runtime

Linux exposes the active Device Tree under:

```text
/proc/device-tree/
```

Check:

```bash
ls /proc/device-tree/
```

Search for GPIO nodes:

```bash
find /proc/device-tree -iname "*gpio*"
```

You can also inspect the Device Tree using:

```bash
dtc -I fs -O dts /proc/device-tree
```

For example:

```bash
dtc -I fs -O dts /proc/device-tree > running-device-tree.dts
```

Then:

```bash
grep -i gpio running-device-tree.dts
```

This is useful for checking whether the Device Tree actually running
on the board contains the expected configuration.

---

# 19. GPIO Testing

After boot, verify GPIO controllers:

```bash
gpiodetect
```

Example:

```text
gpiochip0 [stm32-gpio] ...
gpiochip1 [stm32-gpio] ...
```

Display lines:

```bash
gpioinfo
```

Read a GPIO:

```bash
gpioget gpiochip0 <line>
```

Set a GPIO:

```bash
gpioset gpiochip0 <line>=1
```

Monitor events:

```bash
gpiomon gpiochip0 <line>
```

---

# 19.1 LED Test

The application:

```text
application/examples/led_blink.c
```

uses the GPIO abstraction.

Flow:

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
libgpiod
     |
     v
/dev/gpiochipX
     |
     v
GPIO Driver
     |
     v
GPIO Pin
     |
     v
LED
```

Test:

```bash
./led_blink
```

Expected result:

```text
LED ON
LED OFF
LED ON
LED OFF
...
```

---

# 19.2 GPIO Toggle Test

Application:

```text
application/examples/gpio_toggle.c
```

Flow:

```text
gpio_toggle.c
      |
      v
GPIO initialization
      |
      v
GPIO output
      |
      v
Toggle value
      |
      +---- 0
      |
      +---- 1
      |
      +---- 0
      |
      +---- 1
```

---

# 19.3 Button Test

Application:

```text
application/examples/button.c
```

Flow:

```text
Button
   |
   v
GPIO Input
   |
   v
libgpiod
   |
   v
button.c
   |
   v
Print button state
```

Example:

```text
Button released
Button pressed
Button released
```

---

# 19.4 Button Interrupt Test

Application:

```text
application/examples/button_irq.c
```

Flow:

```text
Button
   |
   v
GPIO edge
   |
   v
GPIO controller
   |
   v
Linux IRQ
   |
   v
GPIO character device
   |
   v
libgpiod
   |
   v
button_irq.c
```

Monitor events:

```bash
gpiomon gpiochip0 <line>
```

Expected:

```text
Falling edge
Rising edge
```

---

# 20. Troubleshooting

## 20.1 GPIO Chip Not Available

Check:

```bash
ls /dev/gpiochip*
```

If no GPIO chip is available:

```bash
dmesg | grep -i gpio
```

Check:

```bash
dmesg | grep -i pinctrl
```

Possible causes:

```text
GPIO driver not enabled
Incorrect Device Tree
Incorrect kernel configuration
Pin controller not initialized
GPIO controller disabled
```

---

## 20.2 GPIO Pin Not Visible

Run:

```bash
gpioinfo
```

Check whether the expected GPIO line exists.

Then inspect:

```bash
cat /sys/kernel/debug/gpio
```

Possible cause:

```text
GPIO already claimed by another driver.
```

---

## 20.3 Wrong GPIO Pin

A common mistake is confusing:

```text
Physical connector pin
```

with:

```text
Linux GPIO line number
```

They are not necessarily the same.

Always determine:

```text
Board connector pin
       |
       v
STM32 GPIO port/pin
       |
       v
GPIO controller
       |
       v
GPIO line/offset
```

Use the board schematic and the active Device Tree to verify the mapping.

---

## 20.4 Pinmux Conflict

If a GPIO does not behave correctly, check whether the same physical
pin is configured for another peripheral.

Example:

```text
Expected:

GPIO
 |
 +---- LED

Actual:

SPI
 |
 +---- Same physical pin
```

The pin cannot simultaneously behave as the intended GPIO and another
alternate function.

Check the active pinctrl configuration.

---

## 20.5 Button Always Reads the Same Value

Check:

```text
Pull-up
Pull-down
Button wiring
GPIO polarity
Pinmux
```

For an active-low button:

```text
Released = 1
Pressed  = 0
```

For an active-high button:

```text
Released = 0
Pressed  = 1
```

---

# 21. Complete Device Tree Flow

The complete project flow is:

```text
                  stm32mp157-gpio-test.dts
                              |
                              v
                       Device Tree Compiler
                              |
                              v
                   stm32mp157-gpio-test.dtb
                              |
                              v
                         Boot Media
                              |
                              v
                           U-Boot
                              |
                              v
                        Linux Kernel
                              |
                              v
                   Device Tree Parser
                              |
             +----------------+----------------+
             |                                 |
             v                                 v
        Pin Controller                    GPIO Node
             |                                 |
             v                                 v
         Pinmux Setup                    GPIO Driver
                                               |
                                               v
                                      Linux GPIO Subsystem
                                               |
                              +----------------+----------------+
                              |                                 |
                              v                                 v
                       /sys/class/gpio                    /dev/gpiochipX
                           Legacy                           Character
                           Sysfs                            Interface
                              |                                 |
                              v                                 v
                       gpio-sysfs.c                     libgpiod.c
                              |                                 |
                              +----------------+----------------+
                                               |
                                               v
                                      Application Layer
                                               |
                         +---------------------+--------------------+
                         |                     |                    |
                         v                     v                    v
                    led_blink.c          gpio_toggle.c          button.c
                                                                      |
                                                                      v
                                                               button_irq.c
                         |                     |                    |
                         +---------------------+--------------------+
                                               |
                                               v
                                      STM32MP157 GPIO Pin
                                               |
                              +----------------+----------------+
                              |                                 |
                              v                                 v
                             LED                             Button
```

---

# 22. Device Tree Design for This Project

The Device Tree portion of this project should follow this design:

```text
STM32MP157-DK2
      |
      +-- Existing STM32MP157 board DTS
      |
      +-- GPIO Controller
      |
      +-- Pin Controller
      |
      +-- GPIO Test Pin
      |
      +-- LED GPIO
      |
      +-- Button GPIO
      |
      +-- Button Interrupt
      |
      +-- GPIO polarity
      |
      +-- Pull configuration
```

The application layer should **not** directly configure the STM32 GPIO
registers.

Instead:

```text
Device Tree
     |
     v
Hardware Configuration

Application
     |
     v
libgpiod / GPIO API
     |
     v
Linux GPIO subsystem
     |
     v
GPIO Driver
     |
     v
Hardware
```

This provides a clean separation between:

```text
Hardware Description
        |
        v
Device Tree

Hardware Control
        |
        v
Linux GPIO Driver

User Application
        |
        v
libgpiod / GPIO API
```

---

# 23. Recommended Project Development Flow

For the STM32MP157-DK2 GPIO project, follow this order:

```text
Step 1
 |
 v
Identify physical GPIO pins
 |
 v
Step 2
 |
 v
Check STM32MP157 schematic
 |
 v
Step 3
 |
 v
Configure pinctrl
 |
 v
Step 4
 |
 v
Configure GPIO nodes
 |
 v
Step 5
 |
 v
Configure LED / Button
 |
 v
Step 6
 |
 v
Configure interrupt if required
 |
 v
Step 7
 |
 v
Build DTB
 |
 v
Step 8
 |
 v
Deploy DTB
 |
 v
Step 9
 |
 v
Boot board
 |
 v
Step 10
 |
 v
Verify /proc/device-tree
 |
 v
Step 11
 |
 v
Run gpiodetect
 |
 v
Step 12
 |
 v
Run gpioinfo
 |
 v
Step 13
 |
 v
Test gpioset/gpioget/gpiomon
 |
 v
Step 14
 |
 v
Run project applications
 |
 v
LED / Button validation
```

---

# 24. Key Interview Explanation

For an interview, the Device Tree flow can be explained as:

> "In the STM32MP157-DK2 GPIO project, I use Device Tree to describe the
> GPIO controller, pin multiplexing, GPIO pins, polarity, pull
> configuration and interrupt information. During boot, U-Boot passes
> the DTB to the Linux kernel. The kernel parses the Device Tree and
> probes the STM32 GPIO and pinctrl drivers. The GPIO controller is then
> registered with the Linux GPIO subsystem and exposed through the GPIO
> character-device interface. My user-space applications use libgpiod
> to access `/dev/gpiochipX`, while the legacy implementation uses the
> Sysfs GPIO interface."

---

# 25. Important Architecture Principle

The project follows this separation:

```text
+------------------------------------------------+
|                Application Layer               |
|                                                |
| led_blink.c / button.c / button_irq.c          |
+------------------------------------------------+
                       |
                       v
+------------------------------------------------+
|                 GPIO API Layer                 |
|                                                |
| gpio-common.c                                  |
+------------------------------------------------+
                       |
              +--------+--------+
              |                 |
              v                 v
+--------------------+   +----------------------+
|   Sysfs Backend    |   |   libgpiod Backend   |
| gpio-sysfs.c       |   | gpio-libgpiod.c      |
+--------------------+   +----------------------+
              |                 |
              v                 v
       /sys/class/gpio     /dev/gpiochipX
              |                 |
              +--------+--------+
                       |
                       v
+------------------------------------------------+
|             Linux GPIO Subsystem              |
+------------------------------------------------+
                       |
                       v
+------------------------------------------------+
|          STM32 GPIO / Pinctrl Driver           |
+------------------------------------------------+
                       |
                       v
+------------------------------------------------+
|             STM32MP157 Hardware               |
+------------------------------------------------+
```

The Device Tree belongs primarily to the **hardware description and
kernel-driver layer**, not to the application layer.

---

```
```

