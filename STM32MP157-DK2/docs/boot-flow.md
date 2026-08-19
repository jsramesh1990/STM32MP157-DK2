For your **`STM32MP157-DK2`** project, `docs/boot-flow.md` should explain the complete boot-to-GPIO-test flow, specifically for the STM32MP157-DK2 Linux/Yocto environment.

Create:

```text
STM32MP157-DK2/docs/boot-flow.md
```

with the following content:

````markdown
# STM32MP157-DK2 Boot Flow

## Table of Contents

- [1. Overview](#1-overview)
- [2. Hardware Boot Architecture](#2-hardware-boot-architecture)
- [3. Complete Boot Flow](#3-complete-boot-flow)
- [4. ROM Boot](#4-rom-boot)
- [5. TF-A / First-Stage Bootloader](#5-tf-a--first-stage-bootloader)
- [6. U-Boot](#6-u-boot)
- [7. Linux Kernel](#7-linux-kernel)
- [8. Device Tree](#8-device-tree)
- [9. Root Filesystem](#9-root-filesystem)
- [10. GPIO Driver Initialization](#10-gpio-driver-initialization)
- [11. Application Startup](#11-application-startup)
- [12. GPIO Application Flow](#12-gpio-application-flow)
- [13. Complete Project Flow](#13-complete-project-flow)
- [14. Debugging Boot Flow](#14-debugging-boot-flow)
- [15. Boot Flow Diagram](#15-boot-flow-diagram)

---

# 1. Overview

The STM32MP157-DK2 project uses the STM32MP157 application processor to
demonstrate Linux GPIO control using both the legacy Sysfs GPIO interface
and the modern libgpiod GPIO character-device interface.

The complete software stack is:

```text
STM32MP157-DK2 Hardware
        |
        v
STM32MP157 ROM Code
        |
        v
TF-A / Boot Firmware
        |
        v
U-Boot
        |
        v
Linux Kernel
        |
        +-------------------+
        |                   |
        v                   v
Device Tree           Kernel Drivers
        |                   |
        +---------+---------+
                  |
                  v
            GPIO Controller
                  |
                  v
             /dev/gpiochip*
                  |
                  v
          Root Filesystem
                  |
                  v
        GPIO Application
                  |
          +-------+-------+
          |               |
          v               v
       Sysfs          libgpiod
          |               |
          v               v
       GPIO Pin       GPIO Pin
````

---

# 2. Hardware Boot Architecture

The STM32MP157-DK2 is based on the STM32MP157A processor.

The STM32MP157 contains:

```text
+-------------------------------------------------------+
|                  STM32MP157 SoC                       |
|                                                       |
|  +-------------------+     +-----------------------+ |
|  | Cortex-A7 CPU 0   |     | Cortex-A7 CPU 1       | |
|  | Linux Application |     | Linux Application     | |
|  +-------------------+     +-----------------------+ |
|                                                       |
|  +-----------------------------------------------+   |
|  | Cortex-M4                                       |   |
|  | Real-time / Firmware Processing                |   |
|  +-----------------------------------------------+   |
|                                                       |
|  +-----------------------------------------------+   |
|  | GPIO Controllers                               |   |
|  | GPIOA ... GPIOZ                                |   |
|  +-----------------------------------------------+   |
|                                                       |
|  +-----------------------------------------------+   |
|  | DDR / SDMMC / eMMC / USB / Ethernet / I2C     |   |
|  | SPI / UART / ADC / PWM / Timers               |   |
|  +-----------------------------------------------+   |
+-------------------------------------------------------+
```

Linux normally executes on the Cortex-A7 processors.

The Cortex-M4 can be used for independent real-time firmware, but this
GPIO Linux project primarily uses the Cortex-A7 Linux environment.

---

# 3. Complete Boot Flow

The complete boot sequence is:

```text
Power ON
   |
   v
STM32MP157 Boot ROM
   |
   v
Boot Device Detection
   |
   v
TF-A / FSBL
   |
   +--> DDR Initialization
   |
   +--> Clock Initialization
   |
   +--> Security / TrustZone Setup
   |
   v
U-Boot
   |
   +--> Environment
   |
   +--> Storage Detection
   |
   +--> Load Kernel
   |
   +--> Load Device Tree
   |
   +--> Load RootFS information
   |
   v
Linux Kernel
   |
   +--> CPU Initialization
   |
   +--> Memory Management
   |
   +--> Interrupt Controller
   |
   +--> Bus Initialization
   |
   +--> Driver Initialization
   |
   +--> GPIO Controller Driver
   |
   v
Device Tree GPIO Configuration
   |
   v
/dev/gpiochip*
   |
   v
Root Filesystem
   |
   v
systemd / init
   |
   v
GPIO Application
   |
   +--> gpio-sysfs
   |
   +--> gpio-libgpiod
   |
   +--> button
   |
   +--> button_irq
   |
   +--> led_blink
   |
   +--> gpio_toggle
   |
   v
Physical GPIO
```

---

# 4. ROM Boot

After power-on or reset, execution starts inside the STM32MP157 internal
Boot ROM.

The Boot ROM is permanently stored inside the SoC.

Its main responsibilities include:

```text
Power-on
   |
   v
Boot ROM
   |
   +--> Initialize minimum hardware
   |
   +--> Read boot configuration
   |
   +--> Determine boot device
   |
   +--> Access boot media
   |
   v
Load First Stage Bootloader
```

Possible boot sources include board-supported boot media such as:

```text
SD Card
eMMC
Other supported boot interfaces
```

The Boot ROM does not start Linux directly.

Its job is to locate and execute the next boot stage.

---

# 5. TF-A / First-Stage Bootloader

After ROM execution, the STM32MP1 boot architecture can load Trusted
Firmware-A (TF-A) and associated boot firmware.

TF-A performs early platform initialization.

Important responsibilities include:

```text
TF-A
 |
 +--> CPU initialization
 |
 +--> DDR initialization
 |
 +--> Clock configuration
 |
 +--> Power / regulator configuration
 |
 +--> TrustZone configuration
 |
 +--> Secure monitor initialization
 |
 +--> Prepare hand-off to U-Boot
 |
 v
U-Boot
```

DDR initialization is particularly important because Linux and U-Boot
require external DDR memory for normal execution.

Without valid DDR initialization, the later boot stages cannot operate
normally.

---

# 6. U-Boot

U-Boot is the main bootloader used to load and start Linux.

The basic flow is:

```text
TF-A
 |
 v
U-Boot
 |
 +--> Initialize hardware
 |
 +--> Initialize storage
 |
 +--> Read boot environment
 |
 +--> Select boot target
 |
 +--> Load Linux kernel
 |
 +--> Load Device Tree
 |
 +--> Configure kernel boot arguments
 |
 v
Boot Linux
```

Typical boot artifacts are:

```text
Kernel Image
Device Tree Blob (.dtb)
Root Filesystem
```

The Device Tree describes the hardware configuration to Linux.

For this project, the GPIO configuration is provided through:

```text
device-tree/
├── README.md
├── stm32mp157-gpio-test.dts
└── stm32mp157-gpio-test-overlay.dts
```

---

# 7. Linux Kernel

Once U-Boot transfers execution to the Linux kernel, kernel initialization
begins.

Simplified sequence:

```text
U-Boot
  |
  v
Linux Kernel
  |
  +--> Decompress / relocate if required
  |
  +--> Setup CPU
  |
  +--> Setup memory
  |
  +--> Setup scheduler
  |
  +--> Setup interrupts
  |
  +--> Initialize buses
  |
  +--> Parse Device Tree
  |
  +--> Initialize platform devices
  |
  +--> Initialize drivers
  |
  v
Start userspace
```

The kernel uses the Device Tree to discover platform hardware.

For the GPIO project:

```text
Device Tree
     |
     v
STM32 GPIO Controller
     |
     v
Linux GPIO subsystem
```

---

# 8. Device Tree

The Device Tree provides hardware description information to Linux.

This project contains:

```text
device-tree/
├── README.md
├── stm32mp157-gpio-test.dts
└── stm32mp157-gpio-test-overlay.dts
```

The Device Tree can describe:

```text
GPIO controllers
GPIO pins
Pin multiplexing
Pin configuration
LEDs
Buttons
Interrupts
GPIO polarity
Peripheral connections
```

Conceptually:

```text
Device Tree
    |
    +--> GPIO Controller
    |
    +--> pinctrl
    |
    +--> GPIO pins
    |
    +--> LED
    |
    +--> Button
    |
    +--> Interrupt
```

For example:

```dts
led_gpio {
    compatible = "gpio-leds";

    led {
        label = "stm32-led";
        gpios = <&gpioa 13 GPIO_ACTIVE_HIGH>;
        default-state = "off";
    };
};
```

The actual GPIO number and pin must match the STM32MP157-DK2 hardware
and the selected pin configuration.

---

# 9. Root Filesystem

After the kernel initializes the hardware, it mounts the root filesystem.

The root filesystem contains:

```text
/bin
/sbin
/etc
/lib
/usr
/dev
/proc
/sys
```

For this project, the important directories are:

```text
/dev
/sys
```

GPIO interfaces appear through these kernel-created interfaces.

Modern GPIO:

```text
/dev/gpiochip0
/dev/gpiochip1
...
```

Legacy Sysfs, when enabled by the kernel:

```text
/sys/class/gpio/
```

The application binaries are then installed into the target filesystem.

Example:

```text
/usr/bin/gpio-sysfs
/usr/bin/gpio-libgpiod
/usr/bin/led_blink
/usr/bin/gpio_toggle
/usr/bin/button
/usr/bin/button_irq
```

---

# 10. GPIO Driver Initialization

During kernel initialization, the STM32 GPIO controller driver is
registered.

The flow is:

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
Linux GPIO Subsystem
     |
     v
GPIO Chip Registration
     |
     v
/dev/gpiochipX
```

The Linux GPIO subsystem provides a common interface between applications
and hardware-specific GPIO controllers.

Application does not directly access STM32 GPIO registers.

Instead:

```text
Application
     |
     v
libgpiod
     |
     v
/dev/gpiochipX
     |
     v
GPIO Character Device
     |
     v
Linux GPIO Subsystem
     |
     v
STM32 GPIO Driver
     |
     v
STM32 GPIO Registers
     |
     v
Physical GPIO Pin
```

---

# 11. Application Startup

Once Linux userspace starts, the GPIO applications can be executed.

Project applications:

```text
application/
├── examples/
│   ├── button.c
│   ├── button_irq.c
│   ├── gpio_toggle.c
│   └── led_blink.c
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

The architecture separates:

```text
Application Examples
        |
        v
Common GPIO API
        |
   +----+----+
   |         |
   v         v
Sysfs     libgpiod
   |         |
   v         v
/sys      /dev/gpiochip
   |         |
   +----+----+
        |
        v
Linux GPIO subsystem
        |
        v
STM32 GPIO driver
        |
        v
STM32MP157 GPIO hardware
```

---

# 12. GPIO Application Flow

## 12.1 LED Blink

The LED application performs:

```text
led_blink
    |
    v
gpio_init()
    |
    v
Configure GPIO as OUTPUT
    |
    v
Write GPIO = 1
    |
    v
LED ON
    |
    v
Delay
    |
    v
Write GPIO = 0
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

Example:

```bash
./led_blink
```

---

## 12.2 GPIO Toggle

The toggle application changes the current GPIO state.

```text
Read GPIO
   |
   v
Current = 0?
   |
   +---- YES ---> Write 1
   |
   +---- NO ----> Write 0
   |
   v
GPIO toggled
```

Example:

```bash
./gpio_toggle
```

---

## 12.3 Button Input

The button application configures a GPIO as an input.

```text
Button
   |
   v
Physical GPIO
   |
   v
STM32 GPIO Controller
   |
   v
Linux GPIO Driver
   |
   v
libgpiod
   |
   v
button application
   |
   v
Read GPIO
```

Example:

```bash
./button
```

---

## 12.4 Button Interrupt

The interrupt version uses GPIO edge detection.

```text
Button Press
     |
     v
GPIO level changes
     |
     v
STM32 GPIO interrupt
     |
     v
GIC / Interrupt subsystem
     |
     v
Linux GPIO IRQ handling
     |
     v
libgpiod event
     |
     v
button_irq
     |
     v
Application handles event
```

Possible events:

```text
RISING EDGE
FALLING EDGE
BOTH EDGES
```

Example:

```bash
./button_irq
```

---

# 13. Complete Project Flow

The complete STM32MP157-DK2 GPIO project can be represented as:

```text
                   STM32MP157-DK2
                         |
                         v
                  Boot ROM
                         |
                         v
                      TF-A
                         |
                         v
                      U-Boot
                         |
              +----------+----------+
              |                     |
              v                     v
        Linux Kernel            Device Tree
              |                     |
              +----------+----------+
                         |
                         v
                 GPIO Driver
                         |
                         v
                Linux GPIO Core
                         |
             +-----------+-----------+
             |                       |
             v                       v
       /sys/class/gpio          /dev/gpiochipX
         Sysfs                    libgpiod
             |                       |
             +-----------+-----------+
                         |
                         v
                  GPIO Common API
                         |
             +-----------+-----------+
             |           |           |
             v           v           v
         LED Blink    Toggle      Button
                                     |
                                     v
                                Button IRQ
                                     |
                                     v
                              Physical GPIO
```

---

# 14. Debugging Boot Flow

## 14.1 Check U-Boot

Connect the STM32MP157-DK2 serial console.

Typical console configuration:

```text
Baud rate : 115200
Data      : 8 bits
Parity    : None
Stop bits : 1
```

Observe:

```text
ROM
 |
 v
TF-A
 |
 v
U-Boot
 |
 v
Linux
```

---

## 14.2 Check Linux Kernel

After Linux boots:

```bash
uname -a
```

Check kernel messages:

```bash
dmesg | less
```

Search GPIO messages:

```bash
dmesg | grep -i gpio
```

---

## 14.3 Check GPIO Controllers

Use:

```bash
ls /dev/gpiochip*
```

Expected:

```text
/dev/gpiochip0
/dev/gpiochip1
...
```

Check GPIO chips:

```bash
gpiodetect
```

Check GPIO lines:

```bash
gpioinfo
```

---

## 14.4 Check Sysfs

If legacy GPIO Sysfs support is enabled:

```bash
ls /sys/class/gpio/
```

Example:

```text
export
unexport
gpioXX
```

Check GPIO:

```bash
cat /sys/class/gpio/gpioXX/direction
cat /sys/class/gpio/gpioXX/value
```

---

## 14.5 Debug Device Tree

Check the live Device Tree:

```bash
ls /proc/device-tree/
```

GPIO-related nodes can be searched using:

```bash
find /proc/device-tree/ -iname "*gpio*"
```

Device Tree overlays/configuration should be verified against the actual
STM32MP157-DK2 pinmux and board schematic.

---

# 15. Boot Flow Diagram

```text
+------------------------------------------------------+
|                 STM32MP157-DK2                      |
+------------------------------------------------------+
                       |
                       v
+------------------------------------------------------+
|                  STM32 Boot ROM                     |
|                                                      |
|  - Boot source detection                             |
|  - Load boot firmware                                |
+------------------------------------------------------+
                       |
                       v
+------------------------------------------------------+
|                    TF-A                              |
|                                                      |
|  - DDR initialization                                |
|  - Clock initialization                              |
|  - Power configuration                               |
|  - Secure initialization                             |
+------------------------------------------------------+
                       |
                       v
+------------------------------------------------------+
|                   U-Boot                             |
|                                                      |
|  - Load Kernel                                       |
|  - Load DTB                                          |
|  - Configure bootargs                                |
+------------------------------------------------------+
                       |
                       v
+------------------------------------------------------+
|                Linux Kernel                          |
|                                                      |
|  - CPU / Memory                                      |
|  - Interrupts                                        |
|  - Device Tree                                       |
|  - Driver initialization                             |
+------------------------------------------------------+
                       |
                       v
+------------------------------------------------------+
|               STM32 GPIO Driver                      |
+------------------------------------------------------+
                       |
                       v
+------------------------------------------------------+
|               Linux GPIO Subsystem                   |
+------------------------------------------------------+
                       |
             +---------+---------+
             |                   |
             v                   v
       /sys/class/gpio     /dev/gpiochipX
          Sysfs               libgpiod
             |                   |
             +---------+---------+
                       |
                       v
+------------------------------------------------------+
|              GPIO Application Layer                  |
|                                                      |
|  gpio-sysfs.c                                        |
|  gpio-libgpiod.c                                     |
|  gpio-common.c                                       |
|                                                      |
|  Examples:                                           |
|    - led_blink                                       |
|    - gpio_toggle                                     |
|    - button                                          |
|    - button_irq                                      |
+------------------------------------------------------+
                       |
                       v
+------------------------------------------------------+
|               Physical GPIO Pin                      |
|                                                      |
|             LED / Button / External Device          |
+------------------------------------------------------+
```

---

# Project Boot-to-Application Summary

The STM32MP157-DK2 GPIO project follows this sequence:

```text
1. Power ON
       ↓
2. STM32MP157 Boot ROM
       ↓
3. TF-A / First Stage Boot
       ↓
4. DDR + Clock + Platform Initialization
       ↓
5. U-Boot
       ↓
6. Load Linux Kernel
       ↓
7. Load Device Tree
       ↓
8. Linux Kernel Initialization
       ↓
9. STM32 GPIO Driver Probe
       ↓
10. Linux GPIO Subsystem Registration
       ↓
11. /dev/gpiochipX Created
       ↓
12. Root Filesystem Mounted
       ↓
13. GPIO Applications Start
       ↓
14. gpio-common API
       ↓
15. Sysfs / libgpiod
       ↓
16. Linux GPIO Driver
       ↓
17. STM32 GPIO Controller
       ↓
18. Physical GPIO
       ↓
19. LED / Button / External Device
```

---

## Important Note

The GPIO **line numbers, GPIO controller names, pinmux settings, and
physical header pins must be verified against the STM32MP157-DK2
schematic and the board's actual Device Tree**.

Do not directly reuse Raspberry Pi examples such as:

```text
GPIO17
GPIO18
```

as STM32MP157 GPIO numbers. STM32 GPIOs are organized by ports and pins,
and Linux exposes them through GPIO chips and line offsets.

```

This gives your project a proper **boot → kernel → Device Tree → GPIO driver → libgpiod/sysfs → application → physical GPIO** architecture rather than making it just a GPIO application demo.
```

