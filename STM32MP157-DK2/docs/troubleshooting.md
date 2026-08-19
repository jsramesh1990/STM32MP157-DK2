Below is a complete `troubleshooting.md` suitable for your **STM32MP157-DK2** project. It is focused specifically on GPIO, Device Tree, libgpiod, Sysfs, Linux kernel, Yocto, and board bring-up issues.

````markdown
# STM32MP157-DK2 GPIO Project - Troubleshooting Guide

## Table of Contents

1. [Introduction](#1-introduction)
2. [Troubleshooting Flow](#2-troubleshooting-flow)
3. [Board Boot Issues](#3-board-boot-issues)
4. [Device Tree Issues](#4-device-tree-issues)
5. [GPIO Availability Issues](#5-gpio-availability-issues)
6. [Sysfs GPIO Issues](#6-sysfs-gpio-issues)
7. [libgpiod Issues](#7-libgpiod-issues)
8. [GPIO Permission Issues](#8-gpio-permission-issues)
9. [GPIO Direction Issues](#9-gpio-direction-issues)
10. [GPIO Value Issues](#10-gpio-value-issues)
11. [GPIO Interrupt Issues](#11-gpio-interrupt-issues)
12. [LED Issues](#12-led-issues)
13. [Button Issues](#13-button-issues)
14. [Device Tree Overlay Issues](#14-device-tree-overlay-issues)
15. [Kernel Configuration Issues](#15-kernel-configuration-issues)
16. [Application Build Issues](#16-application-build-issues)
17. [libgpiod API Issues](#17-libgpiod-api-issues)
18. [Yocto Build Issues](#18-yocto-build-issues)
19. [Runtime Debugging](#19-runtime-debugging)
20. [GPIO Debugging Commands](#20-gpio-debugging-commands)
21. [Common Error Messages](#21-common-error-messages)
22. [Final Debug Checklist](#22-final-debug-checklist)

---

# 1. Introduction

This document provides troubleshooting procedures for the:

**STM32MP157-DK2 GPIO Control and Simulator Project**

The project supports:

- STM32MP157-DK2 hardware
- Linux GPIO subsystem
- Device Tree configuration
- GPIO controller
- GPIO consumer applications
- Linux GPIO character device
- libgpiod
- Legacy GPIO Sysfs
- GPIO interrupts
- LED control
- Button input
- Yocto-based BSP
- GPIO simulation and testing

The debugging approach follows the complete Linux GPIO flow:

```text
Hardware
   |
   v
STM32MP157 GPIO Controller
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
Sysfs GPIO        /dev/gpiochipN
   |                  |
   v                  v
gpio-sysfs.c      gpio-libgpiod.c
   |                  |
   +--------+---------+
            |
            v
       GPIO Application
            |
            +---- LED
            |
            +---- Button
            |
            +---- Interrupt
````

---

# 2. Troubleshooting Flow

Always debug from the bottom layer to the top layer.

```text
Step 1: Check board boot
        |
        v
Step 2: Check kernel
        |
        v
Step 3: Check Device Tree
        |
        v
Step 4: Check GPIO controller
        |
        v
Step 5: Check /dev/gpiochip*
        |
        v
Step 6: Check GPIO lines
        |
        v
Step 7: Test libgpiod
        |
        v
Step 8: Test application
        |
        v
Step 9: Test LED/Button
        |
        v
Step 10: Debug interrupts
```

Do not start debugging the application before verifying that the GPIO controller and Device Tree are working.

---

# 3. Board Boot Issues

## 3.1 Board Does Not Boot

Check the serial console.

Typical connection:

```text
STM32MP157-DK2
       |
       v
USB-UART
       |
       v
PC
       |
       v
minicom / screen
```

Example:

```bash
sudo minicom -D /dev/ttyUSB0 -b 115200
```

Or:

```bash
screen /dev/ttyUSB0 115200
```

Check boot messages:

```bash
dmesg
```

Look for:

```text
Booting Linux
Linux version
Machine model
Starting kernel
```

---

## 3.2 No Serial Output

Check:

```bash
ls /dev/ttyUSB*
```

or:

```bash
ls /dev/ttyACM*
```

Check USB devices:

```bash
lsusb
```

Verify:

* USB-UART cable
* TX/RX connection
* GND connection
* Baud rate
* Serial port
* Terminal settings

Expected:

```text
115200 baud
8 data bits
No parity
1 stop bit
No flow control
```

---

# 4. Device Tree Issues

Device Tree is one of the most important parts of this project.

The GPIO flow is:

```text
.dts
 |
 v
DTB
 |
 v
Bootloader
 |
 v
Linux Kernel
 |
 v
STM32 GPIO Driver
 |
 v
GPIO Controller
```

---

## 4.1 Check Device Tree Model

Run:

```bash
cat /proc/device-tree/model
```

Expected:

```text
STMicroelectronics STM32MP157...
```

---

## 4.2 Check Device Tree

Check the loaded Device Tree:

```bash
ls /proc/device-tree/
```

GPIO-related nodes can be searched using:

```bash
find /proc/device-tree -iname "*gpio*"
```

---

## 4.3 Check GPIO Controller

Run:

```bash
dmesg | grep -i gpio
```

Expected messages may contain:

```text
gpio gpiochip0
```

or:

```text
stm32-gpio
```

---

## 4.4 Device Tree Changes Not Taking Effect

Possible reasons:

1. Wrong `.dts` file modified
2. DTB not rebuilt
3. Old DTB installed
4. Wrong boot partition
5. U-Boot loading another DTB
6. Overlay not applied
7. Kernel using another Device Tree

Check the bootloader configuration.

Verify the DTB file:

```bash
ls -l /boot/
```

Check:

```bash
dmesg | grep -i "machine"
```

---

# 5. GPIO Availability Issues

First check GPIO chips:

```bash
ls -l /dev/gpiochip*
```

Expected:

```text
/dev/gpiochip0
/dev/gpiochip1
/dev/gpiochip2
...
```

If no devices exist:

```text
/dev/gpiochip*
```

then the GPIO character-device interface is probably not enabled or the GPIO controller has not registered.

---

## 5.1 Install GPIO Tools

On Debian/Ubuntu:

```bash
sudo apt update
sudo apt install gpiod
```

Check:

```bash
which gpiodetect
```

Then:

```bash
gpiodetect
```

Example:

```text
gpiochip0 [gpio-controller] (16 lines)
gpiochip1 [gpio-controller] (16 lines)
```

---

# 6. Sysfs GPIO Issues

> Note: GPIO Sysfs is a legacy interface and is deprecated in modern Linux kernels. The preferred interface for new applications is the GPIO character-device API through libgpiod.

Check:

```bash
ls /sys/class/gpio/
```

Possible output:

```text
export
unexport
gpiochip0
gpiochip1
```

---

## 6.1 Export GPIO

Example:

```bash
echo 17 > /sys/class/gpio/export
```

Check:

```bash
ls /sys/class/gpio/
```

Expected:

```text
gpio17
```

---

## 6.2 Set Direction

Output:

```bash
echo out > /sys/class/gpio/gpio17/direction
```

Input:

```bash
echo in > /sys/class/gpio/gpio18/direction
```

---

## 6.3 Set GPIO Value

Set HIGH:

```bash
echo 1 > /sys/class/gpio/gpio17/value
```

Set LOW:

```bash
echo 0 > /sys/class/gpio/gpio17/value
```

Read:

```bash
cat /sys/class/gpio/gpio17/value
```

---

## 6.4 GPIO Sysfs Does Not Exist

If:

```bash
ls /sys/class/gpio/
```

does not show:

```text
export
unexport
```

then CONFIG_GPIO_SYSFS may not be enabled.

Check:

```bash
zcat /proc/config.gz | grep GPIO_SYSFS
```

or:

```bash
grep GPIO_SYSFS /boot/config-$(uname -r)
```

---

# 7. libgpiod Issues

The modern GPIO interface uses:

```text
/dev/gpiochipN
```

Check:

```bash
ls -l /dev/gpiochip*
```

---

## 7.1 Detect GPIO Chips

Run:

```bash
gpiodetect
```

Example:

```text
gpiochip0 [STM32 GPIO controller] (16 lines)
```

---

## 7.2 Display GPIO Lines

Run:

```bash
gpioinfo
```

or:

```bash
gpioinfo gpiochip0
```

Example:

```text
line 0: "GPIO0" unused input active-high
line 1: "GPIO1" unused input active-high
line 2: "GPIO2" "led" output active-high [used]
```

---

## 7.3 Test GPIO Output

Using `gpioset`:

```bash
gpioset gpiochip0 17=1
```

Set LOW:

```bash
gpioset gpiochip0 17=0
```

---

## 7.4 Read GPIO

```bash
gpioget gpiochip0 18
```

---

## 7.5 GPIO Chip Number Is Wrong

Do not assume:

```text
gpiochip0
```

always represents the GPIO bank you want.

Run:

```bash
gpiodetect
```

Then:

```bash
gpioinfo gpiochip0
gpioinfo gpiochip1
gpioinfo gpiochip2
```

Find the correct GPIO controller and line.

---

# 8. GPIO Permission Issues

Error:

```text
Permission denied
```

Check:

```bash
ls -l /dev/gpiochip*
```

Example:

```text
crw-rw---- 1 root gpio ...
```

Check user groups:

```bash
groups
```

Add the user to GPIO group if available:

```bash
sudo usermod -aG gpio $USER
```

Log out and log in again.

For quick testing:

```bash
sudo gpiodetect
```

---

# 9. GPIO Direction Issues

GPIO must be configured correctly.

Output:

```text
GPIO
 |
 +-- Direction = OUTPUT
 |
 +-- Value = 0/1
```

Input:

```text
GPIO
 |
 +-- Direction = INPUT
 |
 +-- Read value
```

Check with:

```bash
gpioinfo
```

Example:

```text
line 17: "GPIO17" unused output active-high
```

If GPIO is already claimed:

```text
line 17: "GPIO17" "some-driver" output active-high [used]
```

the application may not be able to request it.

---

# 10. GPIO Value Issues

If writing:

```bash
gpioset gpiochip0 17=1
```

but LED does not turn ON, debug in this order.

### Step 1

Check line:

```bash
gpioinfo gpiochip0
```

### Step 2

Check pinmux.

### Step 3

Check Device Tree.

### Step 4

Check physical wiring.

### Step 5

Check LED polarity.

Typical LED connection:

```text
GPIO
 |
220R
 |
LED Anode
 |
LED Cathode
 |
GND
```

---

## Active Low GPIO

Some hardware uses active-low logic.

Then:

```text
GPIO = 0 -> Device ON
GPIO = 1 -> Device OFF
```

Check Device Tree:

```text
GPIO_ACTIVE_LOW
```

Do not assume HIGH always means ON.

---

# 11. GPIO Interrupt Issues

Button interrupt flow:

```text
Button
   |
   v
GPIO Pin
   |
   v
GPIO Controller
   |
   v
IRQ
   |
   v
Linux IRQ subsystem
   |
   v
GPIO driver
   |
   v
Application
```

---

## 11.1 Check IRQ Configuration

Run:

```bash
cat /proc/interrupts
```

Search:

```bash
cat /proc/interrupts | grep -i gpio
```

---

## 11.2 Check Device Tree Interrupt

Example concept:

```dts
button {
    compatible = "gpio-keys";

    button-gpio {
        label = "user-button";
        gpios = <&gpioX Y GPIO_ACTIVE_LOW>;
        linux,code = <KEY_ENTER>;
    };
};
```

---

## 11.3 Edge Detection

For libgpiod, configure:

```text
Rising edge
Falling edge
Both edges
```

Example:

```text
Button released
     |
     v
HIGH
     |
     | press
     v
LOW
```

This produces:

```text
FALLING EDGE
```

Release produces:

```text
RISING EDGE
```

---

# 12. LED Issues

If LED does not blink:

### Check application

```bash
./led_blink
```

### Check GPIO manually

```bash
gpioset gpiochip0 17=1
```

Then:

```bash
gpioset gpiochip0 17=0
```

### Check line

```bash
gpioinfo
```

### Check pinmux

```bash
dmesg | grep -i pinctrl
```

### Check hardware

Verify:

```text
GPIO -> resistor -> LED -> GND
```

Also verify whether the board LED is connected to the GPIO you selected.

Do not assume a physical header GPIO number is the same as the Linux GPIO line offset.

---

# 13. Button Issues

If button input does not work:

Check:

```bash
gpioget gpiochip0 <line>
```

Press the button and run again.

Example:

```bash
gpioget gpiochip0 18
```

Expected:

```text
0
```

Pressed:

```text
1
```

or vice versa depending on active-low configuration.

---

## Button Floating Input

A button input should normally use:

```text
Pull-up
```

or:

```text
Pull-down
```

Without bias:

```text
Button GPIO
     |
     +---- floating
```

This can produce random:

```text
0
1
0
1
```

Use a hardware or software pull resistor.

---

# 14. Device Tree Overlay Issues

Project overlay:

```text
device-tree/
├── stm32mp157-gpio-test.dts
└── stm32mp157-gpio-test-overlay.dts
```

If the overlay is not working:

### Check overlay compilation

```bash
dtc -@ -I dts -O dtb \
    -o stm32mp157-gpio-test-overlay.dtbo \
    stm32mp157-gpio-test-overlay.dts
```

Check generated file:

```bash
ls -l *.dtbo
```

---

## Check Overlay Application

Check bootloader configuration.

Verify the loaded Device Tree from:

```bash
/proc/device-tree/
```

If available:

```bash
find /proc/device-tree -type f | grep gpio
```

---

# 15. Kernel Configuration Issues

Important GPIO kernel configurations include:

```text
CONFIG_GPIOLIB
CONFIG_GPIO_CDEV
CONFIG_GPIO_SYSFS
CONFIG_GPIO_STM32
CONFIG_PINCTRL
CONFIG_PINCTRL_STM32
```

Check:

```bash
zcat /proc/config.gz | grep -E "GPIOLIB|GPIO_CDEV|GPIO_SYSFS|GPIO_STM32"
```

Expected conceptually:

```text
CONFIG_GPIOLIB=y
CONFIG_GPIO_CDEV=y
CONFIG_GPIO_STM32=y
```

Sysfs, if enabled:

```text
CONFIG_GPIO_SYSFS=y
```

---

# 16. Application Build Issues

Project application structure:

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

---

## Compile Sysfs Application

Example:

```bash
gcc \
    -Iapplication/include \
    application/src/gpio-common.c \
    application/src/gpio-sysfs.c \
    application/examples/led_blink.c \
    -o led_blink
```

---

## Compile libgpiod Application

Depending on the installed libgpiod version:

```bash
gcc \
    -Iapplication/include \
    application/src/gpio-common.c \
    application/src/gpio-libgpiod.c \
    application/examples/led_blink.c \
    -lgpiod \
    -o led_blink
```

---

## Check Library

```bash
ldconfig -p | grep gpiod
```

Also:

```bash
pkg-config --modversion libgpiod
```

---

# 17. libgpiod API Issues

libgpiod has different APIs between major versions.

Check:

```bash
pkg-config --modversion libgpiod
```

For example:

```text
1.x
```

and:

```text
2.x
```

have significantly different APIs.

Therefore, do not blindly mix examples from different libgpiod versions.

Check installed headers:

```bash
ls /usr/include/gpiod.h
```

---

# 18. Yocto Build Issues

For the Yocto BSP, first verify:

```bash
bitbake --version
```

Check environment:

```bash
echo $MACHINE
```

Build:

```bash
bitbake <image-name>
```

---

## GPIO Package Missing

If `gpiodetect`, `gpioinfo`, `gpioset`, or `gpioget` are missing, include the appropriate GPIO/libgpiod package in the image.

Example:

```text
IMAGE_INSTALL:append = " libgpiod"
```

Development package:

```text
IMAGE_INSTALL:append = " libgpiod-dev"
```

Exact package names should be verified against the BSP layers being used.

---

## Application Not Included in RootFS

If your application is built but not available on the board:

```bash
which led_blink
```

Check:

```bash
find / -name led_blink 2>/dev/null
```

Verify the Yocto recipe installs:

```text
${bindir}/led_blink
```

---

# 19. Runtime Debugging

## 19.1 Kernel Messages

Run:

```bash
dmesg | tail -50
```

GPIO-specific:

```bash
dmesg | grep -i gpio
```

Pinctrl:

```bash
dmesg | grep -i pinctrl
```

Device Tree:

```bash
dmesg | grep -i device
```

IRQ:

```bash
dmesg | grep -i irq
```

---

## 19.2 GPIO DebugFS

If enabled:

```bash
mount | grep debugfs
```

If not mounted:

```bash
sudo mount -t debugfs none /sys/kernel/debug
```

Check GPIO state:

```bash
cat /sys/kernel/debug/gpio
```

Example:

```text
gpiochip0:
 gpio-17 (                    |led                 ) out hi
 gpio-18 (                    |button              ) in  lo IRQ
```

This is one of the most useful commands for GPIO debugging.

---

# 20. GPIO Debugging Commands

## Board

```bash
uname -a
```

```bash
cat /proc/device-tree/model
```

---

## GPIO Devices

```bash
ls -l /dev/gpiochip*
```

```bash
gpiodetect
```

```bash
gpioinfo
```

---

## GPIO Test

```bash
gpioset gpiochip0 17=1
```

```bash
gpioset gpiochip0 17=0
```

```bash
gpioget gpiochip0 18
```

---

## Sysfs

```bash
ls /sys/class/gpio/
```

```bash
cat /sys/kernel/debug/gpio
```

---

## Kernel

```bash
dmesg | grep -i gpio
```

```bash
dmesg | grep -i pinctrl
```

```bash
dmesg | grep -i irq
```

---

## Interrupts

```bash
cat /proc/interrupts
```

---

## Processes

```bash
ps aux | grep gpio
```

---

## Application

```bash
ldd ./led_blink
```

Check dynamic libraries:

```bash
ldd ./gpio-libgpiod
```

---

# 21. Common Error Messages

## Error: `/dev/gpiochip0: No such file`

Possible causes:

```text
GPIO character device disabled
        |
        +-- CONFIG_GPIO_CDEV missing
        |
        +-- GPIO driver not loaded
        |
        +-- Device Tree problem
        |
        +-- GPIO controller not registered
```

Check:

```bash
dmesg | grep -i gpio
```

---

## Error: `Device or resource busy`

Meaning:

```text
GPIO is already claimed
```

Check:

```bash
gpioinfo
```

Look for:

```text
[used]
```

Find which driver/application owns it.

---

## Error: `Permission denied`

Check:

```bash
ls -l /dev/gpiochip*
```

Check:

```bash
groups
```

Try:

```bash
sudo gpioinfo
```

If sudo works but normal user does not, fix device permissions/group configuration.

---

## Error: `Invalid argument`

Possible causes:

* Wrong GPIO chip
* Wrong line offset
* Invalid direction
* Invalid edge configuration
* Unsupported bias
* Incorrect libgpiod API
* GPIO already configured by another subsystem

Check:

```bash
gpioinfo
```

---

## Error: `GPIO line not found`

Do not assume the GPIO number.

Run:

```bash
gpiodetect
```

Then:

```bash
gpioinfo gpiochip0
```

Find the correct line.

---

## Error: LED Does Not Turn ON

Debug:

```text
Application
    |
    v
libgpiod
    |
    v
/dev/gpiochipN
    |
    v
GPIO line
    |
    v
Pinctrl
    |
    v
Physical pin
    |
    v
LED
```

Test manually:

```bash
gpioset gpiochip0 <line>=1
```

If this fails, the application is not the primary problem.

---

# 22. Final Debug Checklist

Use this checklist before modifying application code.

```text
[ ] Board boots successfully

[ ] Serial console works

[ ] Correct Linux kernel is running

[ ] Correct Device Tree is loaded

[ ] GPIO driver is registered

[ ] /dev/gpiochip* exists

[ ] gpiodetect works

[ ] gpioinfo works

[ ] Correct GPIO chip identified

[ ] Correct GPIO line identified

[ ] GPIO is not already claimed

[ ] Pinmux is configured correctly

[ ] GPIO direction is correct

[ ] GPIO value can be changed

[ ] LED wiring is correct

[ ] Button wiring is correct

[ ] Pull-up/pull-down is configured

[ ] Interrupt configuration is correct

[ ] /proc/interrupts shows expected IRQ

[ ] libgpiod library is installed

[ ] Application links against libgpiod

[ ] Application has required permissions

[ ] Yocto image contains required packages

[ ] Application is installed in RootFS

[ ] Runtime logs are checked

[ ] Hardware behavior is verified
```

---

# Recommended Debug Sequence

For this STM32MP157-DK2 project, use the following exact sequence:

```bash
# 1. Check board
uname -a

# 2. Check Device Tree
cat /proc/device-tree/model

# 3. Check GPIO driver
dmesg | grep -i gpio

# 4. Check GPIO devices
ls -l /dev/gpiochip*

# 5. Detect GPIO controllers
gpiodetect

# 6. Inspect GPIO lines
gpioinfo

# 7. Test output
gpioset gpiochip0 <LINE>=1

# 8. Test output LOW
gpioset gpiochip0 <LINE>=0

# 9. Test input
gpioget gpiochip0 <LINE>

# 10. Check kernel GPIO state
cat /sys/kernel/debug/gpio

# 11. Check interrupts
cat /proc/interrupts

# 12. Run project application
./led_blink

# 13. Run button application
./button

# 14. Run interrupt application
./button_irq
```

The most important rule is:

```text
Do not debug the application first.

Hardware
   ↓
Device Tree
   ↓
Kernel Driver
   ↓
GPIO Subsystem
   ↓
/dev/gpiochipN
   ↓
libgpiod
   ↓
Application
   ↓
LED/Button
```

If a lower layer is broken, every layer above it will also appear to be broken.

```
```

