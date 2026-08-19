Below is a complete `STM32MP157-DK2/docs/stm32mp157-gpio.md` suitable for your project. It focuses specifically on how GPIO works on the STM32MP157-DK2 from **hardware → Device Tree → Linux GPIO subsystem → libgpiod/sysfs → application → testing**.

# STM32MP157-DK2 GPIO Guide

## 1. Overview

The STM32MP157-DK2 GPIO subsystem provides a Linux-based interface for controlling and monitoring GPIO pins.

This project demonstrates GPIO control using:

* STM32MP157-DK2 hardware
* STM32MP157 GPIO controller
* Linux GPIO subsystem
* Device Tree
* GPIO kernel drivers
* `libgpiod`
* Legacy GPIO Sysfs interface
* C application layer
* LED and button examples
* GPIO interrupt/event detection

The overall objective is to provide a complete GPIO software stack that can be used for development, testing, debugging, and learning.

---

# 2. STM32MP157 GPIO Architecture

The STM32MP157 SoC contains multiple GPIO controller banks.

GPIO pins are organized into GPIO ports such as:

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

Each GPIO controller manages multiple GPIO lines.

The Linux system exposes these controllers through the GPIO subsystem.

### High-level architecture

```text
+-------------------------------------------------------+
|                 User Application                      |
|                                                       |
|  led_blink.c     gpio_toggle.c     button.c           |
|  button_irq.c                                        |
+--------------------------+----------------------------+
                           |
                           v
+-------------------------------------------------------+
|              Application GPIO API                    |
|                                                       |
| gpio-common.c                                         |
| gpio-libgpiod.c                                       |
| gpio-sysfs.c                                          |
+--------------------------+----------------------------+
                           |
              +------------+-------------+
              |                          |
              v                          v
       libgpiod API                GPIO Sysfs
       /dev/gpiochipX             /sys/class/gpio
              |                          |
              +------------+-------------+
                           |
                           v
+-------------------------------------------------------+
|              Linux GPIO Subsystem                    |
|                                                       |
| GPIO character device                                |
| GPIO descriptors                                     |
| GPIO consumers                                       |
| GPIO IRQ handling                                    |
+--------------------------+----------------------------+
                           |
                           v
+-------------------------------------------------------+
|              STM32 GPIO Driver                       |
|                                                       |
| stm32_gpio driver                                    |
| pin configuration                                    |
| GPIO direction                                       |
| GPIO value                                           |
| interrupt handling                                   |
+--------------------------+----------------------------+
                           |
                           v
+-------------------------------------------------------+
|                Device Tree                           |
|                                                       |
| GPIO controller                                      |
| GPIO pins                                            |
| pinctrl configuration                                |
| GPIO interrupts                                     |
+--------------------------+----------------------------+
                           |
                           v
+-------------------------------------------------------+
|              STM32MP157 Hardware                    |
|                                                       |
| GPIOA ... GPIOK                                     |
| LED                                                  |
| Button                                               |
+-------------------------------------------------------+
```

---

# 3. GPIO Software Flow

The complete GPIO flow is:

```text
Power ON
   |
   v
BootROM
   |
   v
TF-A
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
Linux GPIO Subsystem
   |
   +------------------+
   |                  |
   v                  v
libgpiod             Sysfs
   |                  |
   +--------+---------+
            |
            v
      User Application
            |
      +-----+------+
      |            |
      v            v
     LED         Button
```

---

# 4. GPIO Hardware

The STM32MP157 GPIO controller provides the following basic functionality:

```text
GPIO Pin
   |
   +-- Input
   |
   +-- Output
   |
   +-- Pull-up
   |
   +-- Pull-down
   |
   +-- Push-pull
   |
   +-- Open-drain
   |
   +-- Interrupt
   |
   +-- Alternate Function
```

A GPIO can therefore be used for:

* LED control
* Button input
* Interrupt generation
* Reset control
* Enable signals
* Chip-select signals
* Hardware status signals
* Peripheral alternate functions

---

# 5. GPIO Controller in Device Tree

The STM32 GPIO controllers are described in the Device Tree.

A simplified representation is:

```dts
gpioa: gpio@50002000 {
    compatible = "st,stm32mp1-gpio";
    gpio-controller;
    #gpio-cells = <2>;
    reg = <0x50002000 0x400>;
    interrupts = <0 11 4>;
};
```

The actual address, interrupt number, and configuration depend on the STM32MP157 device tree supplied by the board BSP.

The important properties are:

```dts
compatible = "st,stm32mp1-gpio";
gpio-controller;
#gpio-cells = <2>;
reg = <...>;
interrupts = <...>;
```

---

# 6. GPIO Controller Registration

During Linux kernel boot:

```text
Device Tree
     |
     v
Platform Device
     |
     v
STM32 GPIO Driver
     |
     v
GPIO Controller Registration
     |
     v
/dev/gpiochip0
/dev/gpiochip1
/dev/gpiochip2
...
```

The exact `/dev/gpiochipN` numbering should **not** be hard-coded without checking the target board.

Use:

```bash
gpiodetect
```

Example:

```text
gpiochip0 [gpioa] (16 lines)
gpiochip1 [gpiob] (16 lines)
gpiochip2 [gpioc] (16 lines)
...
```

Check detailed GPIO information:

```bash
gpioinfo
```

---

# 7. Linux GPIO Subsystem

The Linux GPIO subsystem provides a common framework between:

```text
User/Application
       |
       v
GPIO API
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

The application does not directly access STM32 GPIO registers.

Instead:

```text
Application
     |
     v
libgpiod / GPIO API
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

This provides hardware abstraction.

---

# 8. GPIO Character Device

Modern Linux GPIO control uses GPIO character devices:

```text
/dev/gpiochip0
/dev/gpiochip1
/dev/gpiochip2
...
```

Applications communicate with the GPIO subsystem through these devices using `libgpiod`.

Example:

```bash
ls -l /dev/gpiochip*
```

Example:

```text
/dev/gpiochip0
/dev/gpiochip1
/dev/gpiochip2
/dev/gpiochip3
```

---

# 9. libgpiod Interface

The project uses `libgpiod` as the preferred modern GPIO interface.

Basic flow:

```text
Application
     |
     v
gpiod_chip_open()
     |
     v
GPIO Chip
     |
     v
gpiod_line_request()
     |
     v
GPIO Line
     |
     v
gpiod_line_set_value()
```

Example:

```c
struct gpiod_chip *chip;

chip = gpiod_chip_open("/dev/gpiochip0");
```

Request a GPIO:

```c
struct gpiod_line *line;

line = gpiod_chip_get_line(chip, line_offset);
```

Configure output:

```c
gpiod_line_request_output(
    line,
    "stm32-gpio-test",
    0
);
```

Set output:

```c
gpiod_line_set_value(line, 1);
```

Read input:

```c
int value;

value = gpiod_line_get_value(line);
```

Release:

```c
gpiod_line_release(line);
gpiod_chip_close(chip);
```

---

# 10. Sysfs GPIO Interface

The project also supports the legacy GPIO Sysfs interface.

Typical interface:

```text
/sys/class/gpio/
```

Example:

```bash
ls /sys/class/gpio/
```

Export GPIO:

```bash
echo 17 > /sys/class/gpio/export
```

Configure output:

```bash
echo out > /sys/class/gpio/gpio17/direction
```

Set value:

```bash
echo 1 > /sys/class/gpio/gpio17/value
```

Read value:

```bash
cat /sys/class/gpio/gpio17/value
```

Unexport:

```bash
echo 17 > /sys/class/gpio/unexport
```

> Sysfs GPIO is considered a legacy interface on modern Linux systems. For new applications, `libgpiod`/GPIO character-device APIs should generally be preferred.

---

# 11. GPIO Direction

GPIO supports two primary directions.

## Output

```text
Application
     |
     v
GPIO OUTPUT
     |
     +---- 0
     |
     +---- 1
```

Example:

```bash
gpioset gpiochip0 5=1
```

Typical usage:

```text
LED
Reset
Enable
Power control
Chip select
```

---

## Input

```text
External signal
      |
      v
GPIO INPUT
      |
      v
Application
```

Example:

```bash
gpioget gpiochip0 6
```

Typical usage:

```text
Button
Sensor status
Interrupt
Hardware fault
Ready/busy signal
```

---

# 12. GPIO Pull Configuration

GPIO inputs can require bias configuration.

Typical options:

```text
Pull-up
Pull-down
No pull
```

Conceptually:

```text
        VDD
         |
       Pull-up
         |
         +------ GPIO
         |
       Button
         |
        GND
```

With pull-up:

```text
Button released = 1
Button pressed  = 0
```

This is commonly used for button input.

---

# 13. GPIO Interrupt

GPIO interrupts allow the processor to respond to a hardware signal change.

Example:

```text
Button
   |
   v
GPIO Input
   |
   v
GPIO Controller
   |
   v
Interrupt
   |
   v
Linux IRQ subsystem
   |
   v
GPIO Driver
   |
   v
Application event
```

Supported edge types can include:

```text
Rising edge
Falling edge
Both edges
```

Example:

```text
0 -----> 1
   Rising Edge
```

and:

```text
1 -----> 0
   Falling Edge
```

---

# 14. Button Interrupt Example

The project contains:

```text
application/examples/button_irq.c
```

The application configures the GPIO as an input and waits for an edge event.

Conceptually:

```c
GPIO INPUT
     |
     v
Configure edge
     |
     v
Wait for event
     |
     v
GPIO transition
     |
     v
Kernel generates event
     |
     v
Application receives event
```

Example output:

```text
GPIO button interrupt test

Waiting for button event...

Button pressed
GPIO value = 0

Button released
GPIO value = 1
```

---

# 15. LED Control

The project uses GPIO output for LED control.

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
/dev/gpiochipX
     |
     v
Linux GPIO subsystem
     |
     v
STM32 GPIO driver
     |
     v
GPIO pin
     |
     v
LED
```

Basic sequence:

```text
GPIO = 1
   |
   v
LED ON
```

Then:

```text
GPIO = 0
   |
   v
LED OFF
```

---

# 16. GPIO Toggle

The project contains:

```text
application/examples/gpio_toggle.c
```

The application repeatedly changes the GPIO state:

```text
0
|
v
1
|
v
0
|
v
1
|
v
...
```

Example:

```text
GPIO Toggle Test

GPIO -> HIGH
GPIO -> LOW
GPIO -> HIGH
GPIO -> LOW
```

This is useful for validating:

* GPIO output
* Linux GPIO subsystem
* Device Tree
* Driver operation
* Hardware signal generation

---

# 17. GPIO Blink Test

The project contains:

```text
application/examples/led_blink.c
```

Typical flow:

```text
Initialize GPIO
     |
     v
Configure OUTPUT
     |
     v
Set HIGH
     |
     v
Delay
     |
     v
Set LOW
     |
     v
Delay
     |
     v
Repeat
```

Example:

```text
LED ON
  |
  | 500 ms
  v
LED OFF
  |
  | 500 ms
  v
LED ON
  |
  v
Repeat
```

---

# 18. Application Directory Structure

Your STM32MP157-DK2 project can be organized as:

```text
STM32MP157-DK2/
│
├── application/
│   │
│   ├── examples/
│   │   ├── button.c
│   │   ├── button_irq.c
│   │   ├── gpio_toggle.c
│   │   └── led_blink.c
│   │
│   ├── include/
│   │   ├── gpio_common.h
│   │   ├── gpio_libgpiod.h
│   │   └── gpio_sysfs.h
│   │
│   └── src/
│       ├── gpio-common.c
│       ├── gpio-libgpiod.c
│       └── gpio-sysfs.c
│
├── configs/
│   ├── gpio-test-config.json
│   └── stm32mp157_gpio_defconfig
│
├── device-tree/
│   ├── README.md
│   ├── stm32mp157-gpio-test.dts
│   └── stm32mp157-gpio-test-overlay.dts
│
├── docs/
│   ├── architecture.md
│   ├── boot-flow.md
│   ├── device-tree.md
│   ├── gpio-driver.md
│   ├── gpio-libgpiod.md
│   ├── gpio-linux-subsystem.md
│   ├── gpio-sysfs.md
│   └── stm32mp157-gpio.md
│
├── scripts/
│   ├── build.sh
│   ├── clean.sh
│   ├── deploy.sh
│   ├── flash_sd.sh
│   ├── run.sh
│   └── test.sh
│
├── Makefile
├── CMakeLists.txt
└── README.md
```

---

# 19. Device Tree GPIO Configuration

A GPIO test node can be created in the board Device Tree.

Example concept:

```dts
gpio_test {
    compatible = "stm32,gpio-test";

    led-gpios = <&gpioa 5 GPIO_ACTIVE_HIGH>;
    button-gpios = <&gpioc 13 GPIO_ACTIVE_LOW>;

    status = "okay";
};
```

The actual GPIO controller and pin numbers must match the STM32MP157-DK2 schematic and the base Device Tree.

---

# 20. GPIO Active High / Active Low

GPIO polarity is important.

### Active High

```text
GPIO = 1
   |
   v
Device ON
```

Device Tree:

```dts
GPIO_ACTIVE_HIGH
```

### Active Low

```text
GPIO = 0
   |
   v
Device ON
```

Device Tree:

```dts
GPIO_ACTIVE_LOW
```

For example, an active-low button:

```dts
button-gpios = <&gpioX Y GPIO_ACTIVE_LOW>;
```

means:

```text
GPIO = 0 → Button pressed
GPIO = 1 → Button released
```

---

# 21. Device Tree to Driver Flow

The Device Tree does not directly control the GPIO.

Instead:

```text
Device Tree
     |
     v
Kernel Device Model
     |
     v
Platform Driver
     |
     v
GPIO Driver
     |
     v
GPIO Controller
```

For a GPIO consumer:

```text
Application / Device Driver
          |
          v
GPIO descriptor
          |
          v
GPIO subsystem
          |
          v
STM32 GPIO controller
```

---

# 22. Pin Control

STM32 GPIO pins may also be configured through the Linux pinctrl subsystem.

Conceptual example:

```dts
pinctrl_gpio_test: gpio-test-pins {
    pins {
        pinmux = <STM32_PINMUX('A', 5, GPIO)>;
        bias-pull-up;
        drive-push-pull;
        slew-rate = <0>;
    };
};
```

Then:

```dts
gpio_test {
    pinctrl-names = "default";
    pinctrl-0 = <&pinctrl_gpio_test>;

    status = "okay";
};
```

The actual pinctrl syntax should follow the STM32MP1 BSP Device Tree version being used.

---

# 23. Kernel Configuration

GPIO functionality requires appropriate kernel configuration.

Example configuration:

```text
CONFIG_GPIOLIB=y
CONFIG_GPIO_SYSFS=y
CONFIG_GPIO_STM32=y
CONFIG_PINCTRL=y
CONFIG_PINCTRL_STM32=y
```

For GPIO character-device support:

```text
CONFIG_GPIO_CDEV=y
```

For interrupts:

```text
CONFIG_GENERIC_IRQ_CHIP=y
```

The exact configuration options depend on the Linux kernel/BSP version.

Your project configuration is stored under:

```text
configs/stm32mp157_gpio_defconfig
```

---

# 24. libgpiod Userspace Package

The target image should contain the GPIO userspace components.

Typical packages:

```text
libgpiod
libgpiod-dev
gpiod
```

For a Yocto image, the corresponding packages can be added to the image configuration.

For example:

```bitbake
IMAGE_INSTALL:append = " libgpiod gpiod"
```

For development:

```bitbake
IMAGE_INSTALL:append = " libgpiod-dev"
```

The exact package names can vary with the Yocto release/layers being used.

---

# 25. Checking GPIO on the Board

After booting STM32MP157-DK2:

```bash
gpiodetect
```

Then:

```bash
gpioinfo
```

Check character devices:

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

Check pinctrl:

```bash
grep -i gpio /sys/kernel/debug/pinctrl/*/pinmux-pins
```

---

# 26. Testing LED GPIO

First identify the correct GPIO chip and line.

```bash
gpiodetect
gpioinfo
```

Then test:

```bash
gpioset gpiochip0 <line>=1
```

Turn it off:

```bash
gpioset gpiochip0 <line>=0
```

For the project application:

```bash
./led_blink
```

Expected:

```text
GPIO initialized
LED ON
LED OFF
LED ON
LED OFF
...
```

---

# 27. Testing Button GPIO

Check the input:

```bash
gpioget gpiochip0 <line>
```

Press the button and repeat:

```bash
gpioget gpiochip0 <line>
```

Expected:

```text
Button released -> 1
Button pressed  -> 0
```

depending on the configured polarity.

---

# 28. Testing GPIO Interrupt

Run:

```bash
./button_irq
```

Expected:

```text
Waiting for GPIO event...

Rising edge detected
GPIO value = 1

Falling edge detected
GPIO value = 0
```

The actual output depends on the button wiring and polarity.

---

# 29. Project Test Matrix

Your project should test GPIO at several levels.

| Test             | Purpose                        |
| ---------------- | ------------------------------ |
| GPIO detection   | Verify controller registration |
| GPIO information | Verify GPIO lines              |
| LED ON/OFF       | Verify output                  |
| GPIO toggle      | Verify repeated output         |
| Button read      | Verify input                   |
| Button IRQ       | Verify interrupt               |
| Pull-up          | Verify bias                    |
| Pull-down        | Verify bias                    |
| Sysfs            | Verify legacy interface        |
| libgpiod         | Verify modern interface        |
| Device Tree      | Verify pin configuration       |
| Kernel logs      | Verify driver initialization   |

---

# 30. Debugging Flow

When GPIO does not work, follow this sequence:

```text
GPIO not working
      |
      v
Check hardware
      |
      v
Check pin mux
      |
      v
Check Device Tree
      |
      v
Check kernel configuration
      |
      v
Check GPIO driver
      |
      v
Check gpiochip
      |
      v
Check GPIO line
      |
      v
Check permissions
      |
      v
Check application
```

Commands:

```bash
gpiodetect
```

```bash
gpioinfo
```

```bash
cat /sys/kernel/debug/gpio
```

```bash
dmesg | grep -i gpio
```

```bash
dmesg | grep -i pinctrl
```

---

# 31. Common GPIO Problems

## GPIO does not appear

Check:

```bash
gpiodetect
```

If the controller is missing:

```text
Device Tree
Kernel configuration
GPIO driver
```

should be checked.

---

## GPIO is busy

Use:

```bash
gpioinfo
```

Look for:

```text
[used]
```

A GPIO may already be consumed by another kernel driver.

---

## Wrong GPIO value

Check:

```text
Pin mux
Polarity
Pull-up/pull-down
External hardware
```

---

## Button constantly reads the same value

Possible causes:

```text
Floating input
Incorrect pull resistor
Incorrect Device Tree polarity
Wrong GPIO line
Hardware wiring issue
```

---

# 32. GPIO Application Design

The project should use a layered design.

```text
+-----------------------------------+
| Example Applications              |
|                                   |
| button.c                          |
| button_irq.c                      |
| gpio_toggle.c                     |
| led_blink.c                       |
+----------------+------------------+
                 |
                 v
+-----------------------------------+
| Common GPIO API                   |
|                                   |
| gpio_init()                       |
| gpio_read()                       |
| gpio_write()                      |
| gpio_cleanup()                    |
+----------------+------------------+
                 |
        +--------+--------+
        |                 |
        v                 v
+---------------+  +---------------+
| libgpiod      |  | Sysfs         |
| Backend       |  | Backend       |
+---------------+  +---------------+
        |                 |
        v                 v
 /dev/gpiochipX      /sys/class/gpio
        |                 |
        +--------+--------+
                 |
                 v
        Linux GPIO Core
                 |
                 v
        STM32 GPIO Driver
                 |
                 v
        STM32MP157 GPIO
```

This architecture makes the project easy to extend.

---

# 33. Why Use a Common GPIO Layer?

Instead of writing application code directly against libgpiod:

```c
gpiod_chip_open();
gpiod_chip_get_line();
gpiod_line_request_output();
```

every application can use:

```c
gpio_init();
gpio_write();
gpio_read();
gpio_cleanup();
```

Advantages:

```text
Application
    |
    v
Common API
    |
    +---- libgpiod
    |
    +---- Sysfs
```

Therefore applications don't need to know which backend is being used.

---

# 34. Project Development Phases

I recommend developing your STM32MP157-DK2 project in these phases.

### Phase 1 — Board Bring-up

```text
Boot STM32MP157-DK2
        |
        v
Linux login
        |
        v
Verify kernel
        |
        v
Verify Device Tree
```

---

### Phase 2 — GPIO Kernel Validation

```bash
gpiodetect
gpioinfo
```

Confirm that GPIO controllers are registered.

---

### Phase 3 — Device Tree

Create:

```text
stm32mp157-gpio-test.dts
```

Configure:

```text
LED GPIO
Button GPIO
Pinctrl
Interrupt
```

---

### Phase 4 — Userspace GPIO

Implement:

```text
gpio-libgpiod.c
gpio-sysfs.c
gpio-common.c
```

---

### Phase 5 — Applications

Implement:

```text
led_blink.c
gpio_toggle.c
button.c
button_irq.c
```

---

### Phase 6 — Testing

Test:

```text
Output
Input
Toggle
Interrupt
Pull-up
Pull-down
Sysfs
libgpiod
```

---

### Phase 7 — Automation

Use:

```text
scripts/build.sh
scripts/deploy.sh
scripts/test.sh
```

to automate the complete workflow.

---

# 35. Final Project Flow

The complete STM32MP157-DK2 GPIO project should finally work like this:

```text
                    STM32MP157-DK2
                           |
                           v
                    Boot Linux BSP
                           |
                           v
                    Device Tree
                           |
                           v
                     Pinctrl Setup
                           |
                           v
                 STM32 GPIO Driver
                           |
                           v
                  Linux GPIO Core
                           |
             +-------------+-------------+
             |                           |
             v                           v
       /dev/gpiochipX              /sys/class/gpio
             |                           |
             v                           v
          libgpiod                    Sysfs
             |                           |
             +-------------+-------------+
                           |
                           v
                    Common GPIO API
                           |
             +-------------+-------------+
             |             |             |
             v             v             v
        LED Blink      GPIO Toggle    Button
                                         |
                                         v
                                    Button IRQ
```

### Key learning outcome

This project gives you a complete embedded Linux GPIO stack:

**STM32MP157 hardware → Device Tree → pinctrl → STM32 GPIO driver → Linux GPIO subsystem → GPIO character device → libgpiod → common C API → LED/Button applications.**

That is the architecture I would use for the **`STM32MP157-DK2` project title**, rather than treating it as only a simple LED/button application.

