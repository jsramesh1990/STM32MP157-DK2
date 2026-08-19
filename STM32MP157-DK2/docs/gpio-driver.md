Below is a complete `STM32MP157-DK2/docs/gpio-driver.md` suitable for your project. It focuses on the **Linux GPIO driver/application flow on STM32MP157-DK2**, including Sysfs, libgpiod, Device Tree, interrupts, and testing.

````markdown
# GPIO Driver and Interface Guide - STM32MP157-DK2

## 1. Overview

The STM32MP157-DK2 GPIO project demonstrates Linux GPIO control using:

- Linux GPIO subsystem
- Device Tree GPIO configuration
- Legacy GPIO Sysfs interface
- Modern libgpiod interface
- GPIO input/output control
- LED control
- Button input
- GPIO interrupt / edge detection
- GPIO toggle and blink applications
- Hardware-independent application structure
- Configuration through JSON
- Kernel configuration through defconfig

The project is designed to provide a complete understanding of how an
application accesses GPIO hardware through the Linux kernel.

---

# 2. Project Architecture

The complete software architecture is:

```text
+-------------------------------------------------------------+
|                     User Applications                       |
|                                                             |
|  gpio_toggle.c    led_blink.c    button.c    button_irq.c |
+-----------------------------+-------------------------------+
                              |
                              v
+-------------------------------------------------------------+
|                    GPIO Common API                          |
|                                                             |
|                    gpio-common.c                            |
|                                                             |
|  gpio_init()                                                |
|  gpio_read()                                                |
|  gpio_write()                                               |
|  gpio_set_direction()                                       |
|  gpio_cleanup()                                             |
+-----------------------------+-------------------------------+
                              |
                 +------------+------------+
                 |                         |
                 v                         v
+--------------------------+   +-----------------------------+
|     GPIO Sysfs API       |   |       libgpiod API          |
|                          |   |                             |
| gpio-sysfs.c             |   | gpio-libgpiod.c             |
|                          |   |                             |
| /sys/class/gpio         |   | /dev/gpiochip*              |
+------------+-------------+   +-------------+---------------+
             |                               |
             +---------------+---------------+
                             |
                             v
+-------------------------------------------------------------+
|                   Linux GPIO Subsystem                      |
|                                                             |
|              GPIO Controller Driver                        |
|                                                             |
|              STM32 GPIO Controller                         |
+-----------------------------+-------------------------------+
                              |
                              v
+-------------------------------------------------------------+
|                     Device Tree                             |
|                                                             |
| stm32mp157-gpio-test.dts                                    |
| stm32mp157-gpio-test-overlay.dts                            |
+-----------------------------+-------------------------------+
                              |
                              v
+-------------------------------------------------------------+
|                     STM32MP157 SoC                          |
|                                                             |
|                  GPIO Hardware                              |
+-------------------------------------------------------------+
                              |
                              v
+-------------------------------------------------------------+
|                    STM32MP157-DK2                           |
|                                                             |
|                 LED / Push Button                           |
+-------------------------------------------------------------+
````

---

# 3. GPIO Software Stack

The GPIO control path can be represented as:

```text
Application
     |
     | GPIO API
     v
Application GPIO Library
     |
     +--------------------------+
     |                          |
     v                          v
Sysfs                     libgpiod
     |                          |
     v                          v
/sys/class/gpio          /dev/gpiochipN
     |                          |
     +------------+-------------+
                  |
                  v
           Linux GPIO Core
                  |
                  v
        STM32 GPIO Driver
                  |
                  v
          STM32 GPIO Controller
                  |
                  v
             GPIO Pin
```

---

# 4. GPIO Driver Components

The project contains three major GPIO software components.

## 4.1 gpio-common.c

Location:

```text
application/src/gpio-common.c
```

This file contains common GPIO functionality.

Responsibilities:

```text
GPIO initialization
GPIO configuration
GPIO read
GPIO write
GPIO direction
GPIO cleanup
Error handling
Interface selection
```

The common layer prevents application programs from directly depending on
the underlying GPIO implementation.

---

# 5. Sysfs GPIO Interface

File:

```text
application/src/gpio-sysfs.c
```

Header:

```text
application/include/gpio_sysfs.h
```

The Sysfs interface exposes GPIO pins through:

```text
/sys/class/gpio/
```

Example:

```text
/sys/class/gpio/
├── export
├── unexport
├── gpio17/
│   ├── direction
│   ├── value
│   └── edge
└── gpio18/
    ├── direction
    ├── value
    └── edge
```

---

# 6. Sysfs GPIO Operations

## 6.1 Export GPIO

```bash
echo 17 > /sys/class/gpio/export
```

The kernel creates:

```text
/sys/class/gpio/gpio17/
```

---

## 6.2 Configure Direction

Output:

```bash
echo out > /sys/class/gpio/gpio17/direction
```

Input:

```bash
echo in > /sys/class/gpio/gpio18/direction
```

---

## 6.3 Write GPIO

Set HIGH:

```bash
echo 1 > /sys/class/gpio/gpio17/value
```

Set LOW:

```bash
echo 0 > /sys/class/gpio/gpio17/value
```

---

## 6.4 Read GPIO

```bash
cat /sys/class/gpio/gpio18/value
```

Example:

```text
0
```

or:

```text
1
```

---

## 6.5 Unexport GPIO

```bash
echo 17 > /sys/class/gpio/unexport
```

---

# 7. libgpiod GPIO Interface

File:

```text
application/src/gpio-libgpiod.c
```

Header:

```text
application/include/gpio_libgpiod.h
```

Modern Linux GPIO applications should preferably use the GPIO character
device interface through libgpiod.

GPIO devices are normally exposed as:

```text
/dev/gpiochip0
/dev/gpiochip1
/dev/gpiochip2
```

Check available GPIO chips:

```bash
ls -l /dev/gpiochip*
```

---

# 8. libgpiod Flow

The application performs:

```text
Open GPIO chip
      |
      v
Select GPIO line
      |
      v
Configure line
      |
      v
Request line
      |
      v
Read / Write GPIO
      |
      v
Release line
      |
      v
Close GPIO chip
```

Example conceptual flow:

```c
chip = open_gpio_chip();

line = get_gpio_line(chip, offset);

request_gpio_output(line);

set_gpio_value(line, 1);

release_gpio_line(line);

close_gpio_chip(chip);
```

---

# 9. GPIO Common Layer

The common GPIO layer provides a unified interface.

Example API:

```c
int gpio_init(gpio_config_t *config);

int gpio_write(gpio_config_t *config, int value);

int gpio_read(gpio_config_t *config);

int gpio_set_direction(gpio_config_t *config,
                       const char *direction);

int gpio_cleanup(gpio_config_t *config);
```

Applications therefore do not need to know whether GPIO access is using
Sysfs or libgpiod.

---

# 10. GPIO Configuration Structure

Example:

```c
typedef struct
{
    int pin;
    int chip;
    int value;

    char direction[16];

    int use_libgpiod;

    char edge[16];

    char bias[16];

} gpio_config_t;
```

Example configuration:

```text
GPIO Pin       : 17
GPIO Chip      : 0
Direction      : output
Initial Value  : 0
Interface      : libgpiod
Edge           : none
```

---

# 11. Device Tree Integration

Device Tree files:

```text
device-tree/
├── README.md
├── stm32mp157-gpio-test.dts
└── stm32mp157-gpio-test-overlay.dts
```

Device Tree describes hardware configuration to the Linux kernel.

The basic relationship is:

```text
Device Tree
     |
     v
GPIO Controller
     |
     v
GPIO Pin
     |
     v
GPIO Consumer
```

---

# 12. GPIO Device Tree Example

Example conceptual node:

```dts
gpio_test
{
    compatible = "stm32,gpio-test";

    led-gpios =
        <&gpioa 5 GPIO_ACTIVE_HIGH>;

    button-gpios =
        <&gpioa 6 GPIO_ACTIVE_LOW>;
};
```

The actual GPIO controller and pin numbers must match the STM32MP157-DK2
hardware configuration being used.

---

# 13. GPIO Flags

Important GPIO flags include:

```text
GPIO_ACTIVE_HIGH
GPIO_ACTIVE_LOW
```

For an active-high LED:

```dts
led-gpios =
    <&gpioa 5 GPIO_ACTIVE_HIGH>;
```

For an active-low button:

```dts
button-gpios =
    <&gpioa 6 GPIO_ACTIVE_LOW>;
```

The active state is important because some hardware is electrically
active when the GPIO output is LOW.

---

# 14. Device Tree Overlay

File:

```text
device-tree/stm32mp157-gpio-test-overlay.dts
```

The overlay allows the GPIO test configuration to be added without
modifying the complete base Device Tree.

Conceptual flow:

```text
Base Device Tree
       |
       +
       |
GPIO Test Overlay
       |
       v
Merged Device Tree
       |
       v
Linux Kernel
       |
       v
GPIO Controller
```

---

# 15. LED GPIO Application

File:

```text
application/examples/led_blink.c
```

Purpose:

```text
Configure GPIO as output
        |
        v
Set GPIO HIGH
        |
        v
LED ON
        |
        v
Delay
        |
        v
Set GPIO LOW
        |
        v
LED OFF
        |
        v
Delay
        |
        v
Repeat
```

Typical application flow:

```c
gpio_init(&config);

while (running)
{
    gpio_write(&config, 1);

    sleep(1);

    gpio_write(&config, 0);

    sleep(1);
}

gpio_cleanup(&config);
```

---

# 16. GPIO Toggle Application

File:

```text
application/examples/gpio_toggle.c
```

Purpose:

```text
Read current GPIO value
        |
        v
Invert value
        |
        v
Write new value
        |
        v
Repeat
```

Example:

```text
Current value = 0
       |
       v
Toggle
       |
       v
New value = 1
```

Next operation:

```text
Current value = 1
       |
       v
Toggle
       |
       v
New value = 0
```

This application is useful for validating GPIO output functionality.

---

# 17. Button Application

File:

```text
application/examples/button.c
```

The button application configures GPIO as an input.

Flow:

```text
Button
   |
   v
GPIO Input
   |
   v
GPIO Driver
   |
   v
Application
   |
   v
Read GPIO value
```

Example logic:

```c
value = gpio_read(&config);

if (value == 0)
{
    printf("Button pressed\n");
}
else
{
    printf("Button released\n");
}
```

---

# 18. Button Interrupt Application

File:

```text
application/examples/button_irq.c
```

This application demonstrates GPIO edge detection.

Flow:

```text
Button Press
      |
      v
GPIO Electrical Transition
      |
      v
GPIO Controller
      |
      v
Linux IRQ subsystem
      |
      v
GPIO Event
      |
      v
Application
      |
      v
Callback / Event Handling
```

Supported events:

```text
Rising Edge
Falling Edge
Both Edges
```

---

# 19. Rising Edge

Example:

```text
0 ────────────┐
              │
              └──────────── 1
                   ^
                   |
               Rising Edge
```

Used when the button transition from LOW to HIGH represents an event.

---

# 20. Falling Edge

```text
1 ────────────┐
              │
              └──────────── 0
                   ^
                   |
               Falling Edge
```

Used when HIGH to LOW represents an event.

---

# 21. Both Edge Detection

```text
0 -> 1 = Rising Edge

1 -> 0 = Falling Edge
```

Both events can be monitored.

This is useful for:

```text
Button press
Button release
Encoder
Digital sensors
Status signals
```

---

# 22. GPIO Interrupt Architecture

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
                 IRQ
                    |
                    v
          Linux IRQ Subsystem
                    |
                    v
             GPIO Subsystem
                    |
                    v
              libgpiod event
                    |
                    v
             button_irq.c
                    |
                    v
             Application
```

---

# 23. Kernel Configuration

File:

```text
configs/stm32mp157_gpio_defconfig
```

The kernel must contain the required GPIO support.

Important configuration areas include:

```text
CONFIG_GPIOLIB
CONFIG_GPIO_CDEV
CONFIG_GPIO_SYSFS
```

Depending on the kernel version and BSP, the exact options may differ.

For modern GPIO applications:

```text
GPIO character device
        +
libgpiod
```

should be preferred.

---

# 24. GPIO Character Device

Modern GPIO access uses:

```text
/dev/gpiochip0
```

instead of:

```text
/sys/class/gpio/
```

Architecture:

```text
Application
     |
     v
libgpiod
     |
     v
/dev/gpiochip0
     |
     v
GPIO character device
     |
     v
Linux GPIO subsystem
     |
     v
STM32 GPIO driver
```

---

# 25. Sysfs vs libgpiod

| Feature            | Sysfs             | libgpiod         |
| ------------------ | ----------------- | ---------------- |
| Interface          | `/sys/class/gpio` | `/dev/gpiochip*` |
| Status             | Legacy            | Modern           |
| API                | File operations   | GPIO library     |
| Events             | Limited           | Supported        |
| GPIO ownership     | Basic             | Better           |
| Line configuration | Limited           | Better           |
| New applications   | Not preferred     | Recommended      |

For this project, both interfaces are implemented to demonstrate the
difference.

---

# 26. GPIO Configuration JSON

File:

```text
configs/gpio-test-config.json
```

Example:

```json
{
    "led": {
        "chip": 0,
        "line": 17,
        "direction": "output",
        "active_low": false
    },

    "button": {
        "chip": 0,
        "line": 18,
        "direction": "input",
        "active_low": true,
        "edge": "both"
    }
}
```

The configuration provides a central place to define GPIO behavior.

---

# 27. Complete GPIO Initialization Flow

```text
Application Start
       |
       v
Read Configuration
       |
       v
Validate GPIO Number
       |
       v
Select GPIO Interface
       |
       +--------------------+
       |                    |
       v                    v
    Sysfs                libgpiod
       |                    |
       +---------+----------+
                 |
                 v
        Configure Direction
                 |
                 v
          Configure Bias
                 |
                 v
          Configure Edge
                 |
                 v
             Request GPIO
                 |
                 v
          GPIO Operation
                 |
          +------+------+
          |             |
          v             v
        READ          WRITE
          |             |
          +------+------+
                 |
                 v
              Cleanup
                 |
                 v
              Exit
```

---

# 28. LED Test Flow

```text
Boot Linux
    |
    v
GPIO driver initialized
    |
    v
GPIO device available
    |
    v
Start led_blink
    |
    v
Open GPIO
    |
    v
Configure OUTPUT
    |
    v
Write HIGH
    |
    v
LED ON
    |
    v
Delay
    |
    v
Write LOW
    |
    v
LED OFF
    |
    v
Repeat
```

---

# 29. Button Polling Flow

```text
Boot
 |
 v
Start button application
 |
 v
Configure GPIO INPUT
 |
 v
Read GPIO
 |
 +------+
 |      |
 v      v
 LOW   HIGH
 |      |
 v      v
Pressed Released
 |
 +------+
 |
 v
Repeat
```

---

# 30. Button Interrupt Flow

```text
Boot
 |
 v
button_irq application
 |
 v
Configure GPIO input
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
GPIO IRQ generated
 |
 v
Linux GPIO subsystem
 |
 v
Event delivered
 |
 v
Application receives event
 |
 v
Print PRESS/RELEASE
 |
 v
Wait for next event
```

---

# 31. Build Flow

The project can be built using the provided build system.

Typical flow:

```text
Source Code
     |
     v
Makefile / CMake
     |
     v
GCC Cross Compiler
     |
     v
Object Files
     |
     v
GPIO Application
     |
     +---------------------+
     |                     |
     v                     v
gpio_toggle           led_blink
button                button_irq
```

---

# 32. Cross Compilation

For STM32MP157-DK2, the application should normally be built using the
cross compiler provided by the STM32MP1 Yocto/SDK environment.

Example:

```bash
source /opt/st/stm32mp*/environment-setup-*
```

Then verify:

```bash
echo $CC
```

Expected output is similar to:

```text
aarch64-...-gcc
```

The exact compiler prefix depends on the STM32MP1 SDK/BSP being used.

---

# 33. Runtime Dependencies

The target RootFS should contain:

```text
GPIO kernel support
libgpiod
libgpiod utilities
shell
C runtime
```

Useful commands:

```bash
gpiodetect
gpioinfo
gpioget
gpioset
```

Check:

```bash
which gpiodetect
```

---

# 34. Target Board Verification

After booting the STM32MP157-DK2:

```bash
uname -a
```

Check GPIO devices:

```bash
ls /dev/gpiochip*
```

Example:

```text
/dev/gpiochip0
/dev/gpiochip1
/dev/gpiochip2
```

Check GPIO controllers:

```bash
gpiodetect
```

Example:

```text
gpiochip0 [gpioa] (...)
gpiochip1 [gpiob] (...)
```

The exact chip numbering depends on the STM32MP157 BSP.

---

# 35. GPIO Information

Use:

```bash
gpioinfo
```

This displays:

```text
GPIO line
GPIO name
Consumer
Direction
Active state
Bias
```

This is very useful when debugging Device Tree and GPIO configuration.

---

# 36. Manual LED Test

For a libgpiod-based system:

```bash
gpioset gpiochip0 17=1
```

LED should turn ON.

Then:

```bash
gpioset gpiochip0 17=0
```

LED should turn OFF.

The actual GPIO chip and line must be verified with:

```bash
gpiodetect
gpioinfo
```

---

# 37. Manual Button Test

Read the button:

```bash
gpioget gpiochip0 18
```

Example:

```text
0
```

Press the button and run again:

```bash
gpioget gpiochip0 18
```

Expected value may change:

```text
1
```

depending on the electrical configuration and active-low setting.

---

# 38. Interrupt Test

For GPIO edge events, use the appropriate libgpiod event tool/version or
the project's `button_irq` application.

Example:

```bash
./button_irq
```

Expected output:

```text
GPIO interrupt test started

Waiting for button event...

Button PRESSED
Button RELEASED
Button PRESSED
Button RELEASED
```

---

# 39. Debugging GPIO

First check:

```bash
gpiodetect
```

Then:

```bash
gpioinfo
```

Then:

```bash
dmesg | grep -i gpio
```

Also:

```bash
cat /proc/interrupts
```

Check GPIO debug information if enabled:

```bash
cat /sys/kernel/debug/gpio
```

---

# 40. Device Tree Debugging

If the GPIO is not available:

```text
Application
     |
     v
GPIO open failed
```

Debug in this order:

```text
1. Check Device Tree
       |
       v
2. Check GPIO controller
       |
       v
3. Check GPIO pin number
       |
       v
4. Check pinctrl configuration
       |
       v
5. Check GPIO ownership
       |
       v
6. Check kernel configuration
       |
       v
7. Check /dev/gpiochip*
```

---

# 41. Pin Control

GPIO functionality also depends on STM32 pinctrl.

Conceptual flow:

```text
STM32 Pin
   |
   v
Pin Multiplexer
   |
   +---- GPIO
   +---- UART
   +---- SPI
   +---- I2C
   +---- PWM
   +---- Other peripheral
```

If the pin is configured for another peripheral, GPIO access may not work.

Therefore:

```text
GPIO problem
    |
    +--> Check GPIO number
    |
    +--> Check pinctrl
    |
    +--> Check alternate function
    |
    +--> Check Device Tree
```

---

# 42. GPIO Ownership

A GPIO may already be claimed by another kernel driver.

Use:

```bash
gpioinfo
```

Look for:

```text
"consumer"
```

Example:

```text
line 17:
    "led"
    output
```

If another driver owns the line, the application may not be able to
request it.

---

# 43. Error Handling

Applications must check every GPIO operation.

Bad:

```c
gpio_write(&config, 1);
```

Preferred:

```c
if (gpio_write(&config, 1) < 0)
{
    perror("GPIO write failed");
    return -1;
}
```

Possible errors:

```text
GPIO not found
GPIO already requested
Permission denied
Invalid GPIO line
Invalid direction
Device not available
Invalid configuration
```

---

# 44. Resource Cleanup

Always release GPIO resources.

Sysfs:

```text
export
   |
   v
use GPIO
   |
   v
unexport
```

libgpiod:

```text
open chip
   |
   v
request line
   |
   v
use GPIO
   |
   v
release line
   |
   v
close chip
```

---

# 45. Application Directory

The application directory contains:

```text
application/
│
├── include/
│   ├── gpio_common.h
│   ├── gpio_libgpiod.h
│   └── gpio_sysfs.h
│
├── src/
│   ├── gpio-common.c
│   ├── gpio-libgpiod.c
│   └── gpio-sysfs.c
│
└── examples/
    ├── button.c
    ├── button_irq.c
    ├── gpio_toggle.c
    └── led_blink.c
```

---

# 46. Header Dependency

```text
gpio_common.h
      |
      +--------------------+
      |                    |
      v                    v
gpio_libgpiod.h      gpio_sysfs.h
      |                    |
      v                    v
gpio-libgpiod.c      gpio-sysfs.c
      |                    |
      +---------+----------+
                |
                v
          gpio-common.c
                |
                v
          Applications
```

---

# 47. Project Execution Flow

The complete project execution is:

```text
STM32MP157-DK2 Power ON
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
GPIO Character Device
          |
          v
RootFS
          |
          v
GPIO Application
          |
          v
libgpiod / Sysfs
          |
          v
GPIO Hardware
          |
          v
LED / Button
```

---

# 48. Testing Strategy

The project should be tested at multiple levels.

## Level 1 - Build Test

```bash
make clean
make
```

Verify all binaries are generated.

---

## Level 2 - Kernel Test

```bash
ls /dev/gpiochip*
```

Verify GPIO character devices exist.

---

## Level 3 - GPIO Controller Test

```bash
gpiodetect
```

Verify GPIO controllers.

---

## Level 4 - GPIO Line Test

```bash
gpioinfo
```

Verify GPIO lines.

---

## Level 5 - LED Test

```bash
./led_blink
```

Verify LED operation.

---

## Level 6 - Toggle Test

```bash
./gpio_toggle
```

Verify GPIO output toggling.

---

## Level 7 - Button Test

```bash
./button
```

Verify GPIO input.

---

## Level 8 - Interrupt Test

```bash
./button_irq
```

Verify GPIO edge events.

---

# 49. Expected Test Matrix

| Test             | Application | Interface        | Expected Result |
| ---------------- | ----------- | ---------------- | --------------- |
| LED ON           | led_blink   | libgpiod         | LED ON          |
| LED OFF          | led_blink   | libgpiod         | LED OFF         |
| Toggle           | gpio_toggle | libgpiod         | GPIO toggles    |
| Button read      | button      | libgpiod         | Input changes   |
| Button IRQ       | button_irq  | libgpiod         | Event detected  |
| Sysfs output     | gpio-sysfs  | Sysfs            | GPIO changes    |
| GPIO detection   | gpiodetect  | Character device | Chip detected   |
| GPIO information | gpioinfo    | Character device | Lines displayed |

---

# 50. Troubleshooting Checklist

## GPIO device missing

```bash
ls /dev/gpiochip*
```

If missing:

```text
Check kernel GPIO configuration
Check GPIO driver
Check Device Tree
Check RootFS
```

---

## GPIO line busy

```bash
gpioinfo
```

Check the consumer.

---

## LED not working

Check:

```text
GPIO number
GPIO polarity
Device Tree
pinctrl
LED wiring
GPIO ownership
```

---

## Button not working

Check:

```text
GPIO direction
pull-up/pull-down
active-low configuration
button wiring
GPIO line
```

---

## Interrupt not working

Check:

```text
GPIO input configuration
edge configuration
IRQ support
Device Tree
libgpiod version
GPIO ownership
```

---

# 51. Security and Permissions

GPIO device access may require appropriate permissions.

Check:

```bash
ls -l /dev/gpiochip*
```

For development:

```bash
sudo ./gpio_toggle
```

For production, configure appropriate udev/group permissions rather than
running applications as root.

---

# 52. Performance Considerations

For basic GPIO control:

```text
Sysfs
  |
  v
File operations
  |
  v
Higher overhead
```

libgpiod:

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
Lower overhead / better GPIO API
```

For high-frequency waveform generation, neither userspace Sysfs nor normal
userspace GPIO toggling should be treated as a deterministic real-time
solution.

For timing-sensitive signals, use appropriate STM32 hardware peripherals
such as:

```text
PWM
Timer
SPI
I2C
Hardware interrupt
```

---

# 53. Production Considerations

For production systems:

1. Prefer libgpiod over GPIO Sysfs.
2. Use Device Tree for hardware description.
3. Avoid hard-coded GPIO assumptions where possible.
4. Use GPIO names/line names when supported.
5. Validate GPIO ownership.
6. Handle errors properly.
7. Release GPIO resources.
8. Configure proper permissions.
9. Avoid busy polling for interrupt-driven events.
10. Use hardware peripherals for deterministic timing.

---

# 54. Learning Flow

This project can be used to learn Linux GPIO from beginner to advanced
level.

```text
GPIO Hardware
      |
      v
STM32 Pinmux
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
      +------------------+
      |                  |
      v                  v
Sysfs              Character Device
      |                  |
      v                  v
gpio-sysfs.c       libgpiod
      |                  |
      +--------+---------+
               |
               v
        Common GPIO API
               |
               v
          Applications
               |
       +-------+-------+
       |       |       |
       v       v       v
      LED    Button   IRQ
```

---

# 55. Interview Explanation

A simple interview explanation for this project is:

> "I developed a Linux GPIO control framework on the STM32MP157-DK2. The
> project uses Device Tree to describe GPIO hardware and supports both the
> legacy Sysfs interface and the modern libgpiod character-device
> interface. I implemented a common GPIO abstraction layer and developed
> applications for LED blinking, GPIO toggling, button input and GPIO
> interrupt handling. I also worked with kernel GPIO configuration,
> pinctrl, GPIO ownership, cross-compilation and target-side debugging."

---

# 56. One-Line Project Flow

```text
Device Tree → STM32 GPIO Driver → Linux GPIO Subsystem → libgpiod/Sysfs → GPIO API → LED/Button Applications
```

---

# 57. Final Project Structure

```text
STM32MP157-DK2/
│
├── application/
│   │
│   ├── include/
│   │   ├── gpio_common.h
│   │   ├── gpio_libgpiod.h
│   │   └── gpio_sysfs.h
│   │
│   ├── src/
│   │   ├── gpio-common.c
│   │   ├── gpio-libgpiod.c
│   │   └── gpio-sysfs.c
│   │
│   └── examples/
│       ├── button.c
│       ├── button_irq.c
│       ├── gpio_toggle.c
│       └── led_blink.c
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
│   └── gpio-driver.md
│
├── scripts/
│   ├── build.sh
│   ├── clean.sh
│   ├── deploy.sh
│   ├── flash_sd.sh
│   ├── run.sh
│   └── test.sh
│
├── CMakeLists.txt
├── Makefile
└── README.md
```

---

## 58. Final Architecture

The final project should demonstrate this complete embedded Linux chain:

```text
                       STM32MP157-DK2
                              |
                              v
                         Boot Process
                              |
                              v
                         Linux Kernel
                              |
                    +---------+---------+
                    |                   |
                    v                   v
                Device Tree         GPIO Config
                    |                   |
                    +---------+---------+
                              |
                              v
                       STM32 GPIO Driver
                              |
                              v
                       Linux GPIO Core
                              |
                    +---------+---------+
                    |                   |
                    v                   v
                  Sysfs              libgpiod
                    |                   |
                    |            /dev/gpiochip*
                    |                   |
                    +---------+---------+
                              |
                              v
                        Common GPIO API
                              |
               +--------------+--------------+
               |              |              |
               v              v              v
          led_blink       gpio_toggle      button
                                             |
                                             v
                                         button_irq
                                             |
                              +--------------+
                              |
                              v
                         GPIO Hardware
                              |
                     +--------+--------+
                     |                 |
                     v                 v
                    LED             BUTTON
```

This makes **STM32MP157-DK2** a proper end-to-end **Embedded Linux GPIO
BSP + Device Tree + GPIO Driver Interface + Userspace Application**
project rather than just a simple GPIO application.

```
```

