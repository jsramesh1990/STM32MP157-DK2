Yes. For your **STM32MP157-DK2** project, keep the `kernel/patches` directory like this:

```text
STM32MP157-DK2/
└── kernel/
    ├── driver/
    │   ├── Makefile
    │   ├── virtual_gpio.c
    │   └── virtual_gpio.h
    │
    └── patches/
        └── README.md
```

For `kernel/patches/README.md`, I recommend documenting **what kernel patches are, why this project may need them, how to create/apply/revert them, and which patches belong to the virtual GPIO driver**.

### `kernel/patches/README.md`

````markdown
# Kernel Patches - STM32MP157-DK2

## 1. Overview

This directory contains Linux kernel patches required for the
STM32MP157-DK2 GPIO project.

The project uses the Linux GPIO subsystem and provides a custom
Virtual GPIO kernel driver.

The patch directory is kept separate from the driver source so that
kernel modifications can be tracked, reviewed, applied, and reverted
independently.

---

## 2. Directory Structure

```text
kernel/
├── driver/
│   ├── Makefile
│   ├── virtual_gpio.c
│   └── virtual_gpio.h
│
└── patches/
    └── README.md
````

Future kernel patches can be stored here:

```text
kernel/
└── patches/
    ├── 0001-add-virtual-gpio-driver.patch
    ├── 0002-enable-gpio-subsystem.patch
    ├── 0003-enable-libgpiod-support.patch
    └── README.md
```

---

# 3. Why Kernel Patches Are Required

Linux kernel development normally involves modifying:

* Kernel source code
* Device Tree
* Kernel configuration
* Drivers
* GPIO subsystem configuration

Instead of directly modifying the kernel source and losing track of
the changes, Git patches can be generated.

A patch provides a reproducible way to apply the same modification
to another kernel source tree.

Example:

```text
Original Kernel
      |
      | Apply Patch
      v
Modified Kernel
      |
      +--> GPIO subsystem
      |
      +--> Virtual GPIO driver
      |
      +--> STM32MP157 GPIO configuration
```

---

# 4. Project Kernel Architecture

The GPIO software stack is:

```text
+------------------------------------------------+
|              User Applications                 |
|                                                |
| led_blink / gpio_toggle / button / PWM        |
+-------------------------+----------------------+
                          |
                          v
+------------------------------------------------+
|                  libgpiod                     |
|                                                |
| /dev/gpiochipX                                |
+-------------------------+----------------------+
                          |
                          v
+------------------------------------------------+
|              Linux GPIO Subsystem             |
|                                                |
| GPIO character device / GPIO framework        |
+-------------------------+----------------------+
                          |
             +------------+------------+
             |                         |
             v                         v
+----------------------+    +----------------------+
| STM32 GPIO Controller|    | Virtual GPIO Driver  |
|                      |    |                      |
| Physical GPIO pins   |    | Software GPIO pins   |
+----------------------+    +----------------------+
             |                         |
             v                         v
       Physical Hardware          Virtual Hardware
```

---

# 5. Kernel Configuration

The Linux kernel must provide GPIO subsystem support.

Important configuration options include:

```text
CONFIG_GPIOLIB=y
CONFIG_GPIO_CDEV=y
CONFIG_GPIO_CDEV_V1=y
```

Depending on the kernel version, additional GPIO options may be
required.

Check the running kernel configuration:

```bash
zcat /proc/config.gz | grep GPIO
```

or:

```bash
cat /boot/config-$(uname -r) | grep GPIO
```

Example:

```text
CONFIG_GPIOLIB=y
CONFIG_GPIO_CDEV=y
CONFIG_GPIO_CDEV_V1=y
```

---

# 6. Building the Virtual GPIO Driver

Go to the driver directory:

```bash
cd kernel/driver
```

Build the kernel module:

```bash
make
```

Expected output:

```text
virtual_gpio.ko
virtual_gpio.o
virtual_gpio.mod.c
virtual_gpio.mod.o
```

The main module is:

```text
virtual_gpio.ko
```

---

# 7. Loading the Driver

Load the module:

```bash
sudo insmod virtual_gpio.ko
```

Check the kernel log:

```bash
dmesg | tail -30
```

Expected messages:

```text
STM32MP157-DK2 Virtual GPIO Driver
virtual_gpio: registered successfully
virtual_gpio: GPIO count = 32
```

---

# 8. Verify GPIO Controller

After loading the module:

```bash
gpiodetect
```

Example:

```text
gpiochip0 [gpio-0-31] (32 lines)
```

The exact gpiochip number depends on the STM32MP157 kernel configuration
and other GPIO controllers present in the system.

Get GPIO information:

```bash
gpioinfo
```

Example:

```text
gpiochipX - 32 lines:
        line   0: "virtual-gpio-0"
        line   1: "virtual-gpio-1"
        line   2: "virtual-gpio-2"
        ...
        line  31: "virtual-gpio-31"
```

---

# 9. Test Virtual GPIO Output

Set GPIO 0 to HIGH:

```bash
gpioset gpiochipX 0=1
```

Set GPIO 0 to LOW:

```bash
gpioset gpiochipX 0=0
```

Read a GPIO:

```bash
gpioget gpiochipX 0
```

The actual gpiochip number must be determined using:

```bash
gpiodetect
```

Do not assume that the virtual controller will always be
`gpiochip0`.

---

# 10. Creating a Kernel Patch

Kernel patches should normally be generated using Git.

Enter the Linux kernel source:

```bash
cd ~/linux
```

Check the repository:

```bash
git status
```

Make the required kernel modification.

For example:

```text
drivers/gpio/
```

or:

```text
drivers/misc/
```

depending on the driver architecture.

Add the modified files:

```bash
git add <modified-file>
```

Create a commit:

```bash
git commit -m "gpio: add virtual GPIO driver"
```

Generate the patch:

```bash
git format-patch -1 HEAD
```

Example output:

```text
0001-gpio-add-virtual-GPIO-driver.patch
```

Copy it into this directory:

```bash
cp 0001-gpio-add-virtual-GPIO-driver.patch \
   STM32MP157-DK2/kernel/patches/
```

---

# 11. Applying a Patch

From the Linux kernel source:

```bash
git apply \
STM32MP157-DK2/kernel/patches/0001-gpio-add-virtual-GPIO-driver.patch
```

Check:

```bash
git status
```

---

# 12. Applying Git Format-Patch

If the patch was generated using:

```bash
git format-patch
```

use:

```bash
git am \
STM32MP157-DK2/kernel/patches/0001-gpio-add-virtual-GPIO-driver.patch
```

This preserves the original commit information.

---

# 13. Checking a Patch Before Applying

Use:

```bash
git apply --check \
STM32MP157-DK2/kernel/patches/0001-gpio-add-virtual-GPIO-driver.patch
```

If there is no output, the patch can normally be applied cleanly.

---

# 14. Reverting a Patch

If the patch was applied using:

```bash
git apply
```

reverse it using:

```bash
git apply -R \
STM32MP157-DK2/kernel/patches/0001-gpio-add-virtual-GPIO-driver.patch
```

If it was applied using:

```bash
git am
```

use:

```bash
git am --abort
```

when resolving an interrupted patch application.

---

# 15. Kernel Patch Workflow

The recommended development workflow is:

```text
        Linux Kernel Source
                |
                v
       Modify Driver / DTS
                |
                v
            Build/Test
                |
                v
          Verify on DK2
                |
                v
          git diff
                |
                v
          git commit
                |
                v
        git format-patch
                |
                v
      kernel/patches/
                |
                v
     Reproducible Kernel Build
```

---

# 16. Device Tree and Kernel Patches

Device Tree changes can also be maintained as patches.

Example:

```text
kernel/patches/
└── 0002-arm-dts-stm32mp157-add-gpio-test.patch
```

The patch may modify:

```text
arch/arm/boot/dts/
```

or the appropriate STM32MP1 Device Tree location for the kernel
version being used.

The Device Tree describes:

* GPIO controllers
* GPIO pins
* GPIO polarity
* GPIO consumers
* LEDs
* Buttons
* Interrupts
* PWM
* Pin multiplexing

---

# 17. GPIO Pin Configuration Flow

For physical GPIO testing:

```text
Device Tree
     |
     v
Pin Controller
     |
     v
STM32 GPIO Controller
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
Application
```

For the virtual GPIO driver:

```text
Virtual GPIO Driver
        |
        v
   gpio_chip
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
   Application
```

---

# 18. Kernel Module vs Built-In Driver

The virtual GPIO driver can be built as a module:

```text
CONFIG_VIRTUAL_GPIO=m
```

Then:

```bash
insmod virtual_gpio.ko
```

Or it can be built directly into the kernel:

```text
CONFIG_VIRTUAL_GPIO=y
```

In that case the driver is initialized during kernel boot.

For development, using:

```text
CONFIG_VIRTUAL_GPIO=m
```

is convenient because the driver can be loaded and unloaded without
rebuilding and rebooting the entire system.

---

# 19. Checking Driver Logs

Check driver messages:

```bash
dmesg | grep virtual_gpio
```

Example:

```text
virtual_gpio: registered successfully
virtual_gpio: GPIO count = 32
virtual_gpio: GPIO 0 configured as OUTPUT
virtual_gpio: GPIO 0 set = 1
```

For more detailed kernel debugging:

```bash
dmesg -w
```

Then run the GPIO application from another terminal.

---

# 20. Patch Naming Convention

Use numbered patch names:

```text
0001-gpio-add-virtual-gpio-driver.patch
0002-arm-dts-stm32mp157-add-gpio-test.patch
0003-gpio-enable-character-device.patch
0004-gpio-add-test-configuration.patch
```

The numbering represents the order in which patches should be applied.

---

# 21. Recommended Patch Organization

For the STM32MP157-DK2 project:

```text
kernel/
└── patches/
    ├── 0001-gpio-add-virtual-gpio-driver.patch
    ├── 0002-gpio-enable-gpio-character-device.patch
    ├── 0003-arm-dts-stm32mp157-gpio-test.patch
    └── README.md
```

Only add a patch when the base kernel actually requires that change.

Do not create patches just for documentation purposes.

---

# 22. Yocto Integration

If this project is built using Yocto, kernel patches can be applied
through a custom kernel recipe.

Example:

```text
meta-stm32mp157-gpio/
└── recipes-kernel/
    └── linux/
        ├── linux-stm32mp/
        │   ├── 0001-gpio-add-virtual-gpio-driver.patch
        │   ├── 0002-arm-dts-stm32mp157-gpio-test.patch
        │   └── linux-stm32mp.bbappend
```

Example `bbappend`:

```bitbake
FILESEXTRAPATHS:prepend := "${THISDIR}/linux-stm32mp:"

SRC_URI += " \
    file://0001-gpio-add-virtual-gpio-driver.patch \
    file://0002-arm-dts-stm32mp157-gpio-test.patch \
"
```

Yocto then applies the patches during the kernel build.

---

# 23. Patch Verification

After applying patches:

```bash
git status
```

Build the kernel:

```bash
make -j$(nproc)
```

Build Device Tree:

```bash
make dtbs -j$(nproc)
```

Build modules:

```bash
make modules -j$(nproc)
```

Install/deploy according to the STM32MP1 BSP build system.

---

# 24. Testing Checklist

After deploying the modified kernel to STM32MP157-DK2:

### Kernel

```bash
uname -a
```

### GPIO subsystem

```bash
gpiodetect
```

### GPIO information

```bash
gpioinfo
```

### Kernel logs

```bash
dmesg | grep -i gpio
```

### Virtual GPIO driver

```bash
lsmod | grep virtual_gpio
```

### GPIO test

```bash
gpioset gpiochipX 0=1
gpioget gpiochipX 0
```

### Application

```bash
./led_blink
```

---

# 25. Troubleshooting

## Patch does not apply

Check:

```bash
git apply --check patch-name.patch
```

If there are conflicts:

```bash
git status
```

Review the affected files and resolve the conflict.

---

## GPIO driver does not appear

Check:

```bash
dmesg | grep virtual_gpio
```

Check:

```bash
lsmod | grep virtual_gpio
```

If necessary:

```bash
sudo insmod virtual_gpio.ko
```

---

## No gpiochip appears

Check:

```bash
gpiodetect
```

and:

```bash
ls -l /dev/gpiochip*
```

Also check:

```bash
dmesg | grep -i gpio
```

---

## Permission denied

Check:

```bash
ls -l /dev/gpiochip*
```

For development, test with:

```bash
sudo gpiodetect
sudo gpioinfo
```

Production systems should use appropriate udev/group permissions rather
than running applications permanently as root.

---

# 26. Important Difference

This project contains two different GPIO paths.

### Physical GPIO

```text
STM32MP157
    |
    v
STM32 GPIO Controller
    |
    v
Device Tree
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
LED / Button
```

### Virtual GPIO

```text
Linux Kernel
    |
    v
virtual_gpio.c
    |
    v
gpio_chip
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
Application
```

The virtual GPIO path is primarily intended for:

* Driver development
* GPIO API learning
* Application testing
* CI testing
* Hardware-independent testing
* Debugging
* Demonstrating the Linux GPIO subsystem

---

# 27. Development Recommendation

For this STM32MP157-DK2 project, develop in the following order:

```text
1. Boot STM32MP157-DK2
          |
          v
2. Verify Linux GPIO subsystem
          |
          v
3. Verify physical GPIO
          |
          v
4. Enable GPIO character device
          |
          v
5. Verify libgpiod
          |
          v
6. Implement virtual_gpio.c
          |
          v
7. Register gpio_chip
          |
          v
8. Verify gpiochip
          |
          v
9. Test gpioset/gpioget
          |
          v
10. Test C applications
          |
          v
11. Add Device Tree configuration
          |
          v
12. Convert modifications into patches
          |
          v
13. Integrate patches into Yocto
          |
          v
14. Final board validation
```

---

# 28. Final Goal

The final project should demonstrate the complete embedded Linux
GPIO stack:

```text
                 STM32MP157-DK2
                        |
                        v
                Bootloader / Linux
                        |
                        v
                  Device Tree
                        |
                        v
                Linux GPIO Core
                        |
             +----------+----------+
             |                     |
             v                     v
      STM32 GPIO Driver     Virtual GPIO Driver
             |                     |
             v                     v
       Physical GPIO          Virtual GPIO
             |                     |
             +----------+----------+
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
          +-------------+-------------+
          |             |             |
          v             v             v
        LED          Button          PWM
```

This gives the project a clear **kernel → GPIO subsystem → driver → Device Tree → libgpiod → application** architecture rather than simply being a collection of GPIO C programs.

```
```




and other GPIO controllers present in the system.

Get GPIO information:

```bash
gpioinfo
```

Example:

```text
gpiochipX - 32 lines:
        line   0: "virtual-gpio-0"
        line   1: "virtual-gpio-1"
        line   2: "virtual-gpio-2"
        ...
        line  31: "virtual-gpio-31"
```

---

# 9. Test Virtual GPIO Output

Set GPIO 0 to HIGH:

```bash
gpioset gpiochipX 0=1
```

Set GPIO 0 to LOW:

```bash
gpioset gpiochipX 0=0
```

Read a GPIO:

```bash
gpioget gpiochipX 0
```

The actual gpiochip number must be determined using:

```bash
gpiodetect
```

Do not assume that the virtual controller will always be
`gpiochip0`.

---

# 10. Creating a Kernel Patch

Kernel patches should normally be generated using Git.

Enter the Linux kernel source:

```bash
cd ~/linux
```

Check the repository:

```bash
git status
```

Make the required kernel modification.

For example:

```text
drivers/gpio/
```

or:

```text
drivers/misc/
```

depending on the driver architecture.

Add the modified files:

```bash
git add <modified-file>
```

Create a commit:

```bash
git commit -m "gpio: add virtual GPIO driver"
```

Generate the patch:

```bash
git format-patch -1 HEAD
```

Example output:

```text
0001-gpio-add-virtual-GPIO-driver.patch
```

Copy it into this directory:

```bash
cp 0001-gpio-add-virtual-GPIO-driver.patch \
   STM32MP157-DK2/kernel/patches/
```

---

# 11. Applying a Patch

From the Linux kernel source:

```bash
git apply \
STM32MP157-DK2/kernel/patches/0001-gpio-add-virtual-GPIO-driver.patch
```

Check:

```bash
git status
```

---

# 12. Applying Git Format-Patch

If the patch was generated using:

```bash
git format-patch
```

use:

```bash
git am \
STM32MP157-DK2/kernel/patches/0001-gpio-add-virtual-GPIO-driver.patch
```

This preserves the original commit information.

---

# 13. Checking a Patch Before Applying

Use:

```bash
git apply --check \
STM32MP157-DK2/kernel/patches/0001-gpio-add-virtual-GPIO-driver.patch
```

If there is no output, the patch can normally be applied cleanly.

---

# 14. Reverting a Patch

If the patch was applied using:

```bash
git apply
```

reverse it using:

```bash
git apply -R \
STM32MP157-DK2/kernel/patches/0001-gpio-add-virtual-GPIO-driver.patch
```

If it was applied using:

```bash
git am
```

use:

```bash
git am --abort
```

when resolving an interrupted patch application.

---

# 15. Kernel Patch Workflow

The recommended development workflow is:

```text
        Linux Kernel Source
                |
                v
       Modify Driver / DTS
                |
                v
            Build/Test
                |
                v
          Verify on DK2
                |
                v
          git diff
                |
                v
          git commit
                |
                v
        git format-patch
                |
                v
      kernel/patches/
                |
                v
     Reproducible Kernel Build
```

---

# 16. Device Tree and Kernel Patches

Device Tree changes can also be maintained as patches.

Example:

```text
kernel/patches/
└── 0002-arm-dts-stm32mp157-add-gpio-test.patch
```

The patch may modify:

```text
arch/arm/boot/dts/
```

or the appropriate STM32MP1 Device Tree location for the kernel
version being used.

The Device Tree describes:

* GPIO controllers
* GPIO pins
* GPIO polarity
* GPIO consumers
* LEDs
* Buttons
* Interrupts
* PWM
* Pin multiplexing

---

# 17. GPIO Pin Configuration Flow

For physical GPIO testing:

```text
Device Tree
     |
     v
Pin Controller
     |
     v
STM32 GPIO Controller
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
Application
```

For the virtual GPIO driver:

```text
Virtual GPIO Driver
        |
        v
   gpio_chip
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
   Application
```

---

# 18. Kernel Module vs Built-In Driver

The virtual GPIO driver can be built as a module:

```text
CONFIG_VIRTUAL_GPIO=m
```

Then:

```bash
insmod virtual_gpio.ko
```

Or it can be built directly into the kernel:

```text
CONFIG_VIRTUAL_GPIO=y
```

In that case the driver is initialized during kernel boot.

For development, using:

```text
CONFIG_VIRTUAL_GPIO=m
```

is convenient because the driver can be loaded and unloaded without
rebuilding and rebooting the entire system.

---

# 19. Checking Driver Logs

Check driver messages:

```bash
dmesg | grep virtual_gpio
```

Example:

```text
virtual_gpio: registered successfully
virtual_gpio: GPIO count = 32
virtual_gpio: GPIO 0 configured as OUTPUT
virtual_gpio: GPIO 0 set = 1
```

For more detailed kernel debugging:

```bash
dmesg -w
```

Then run the GPIO application from another terminal.

---

# 20. Patch Naming Convention

Use numbered patch names:

```text
0001-gpio-add-virtual-gpio-driver.patch
0002-arm-dts-stm32mp157-add-gpio-test.patch
0003-gpio-enable-character-device.patch
0004-gpio-add-test-configuration.patch
```

The numbering represents the order in which patches should be applied.

---

# 21. Recommended Patch Organization

For the STM32MP157-DK2 project:

```text
kernel/
└── patches/
    ├── 0001-gpio-add-virtual-gpio-driver.patch
    ├── 0002-gpio-enable-gpio-character-device.patch
    ├── 0003-arm-dts-stm32mp157-gpio-test.patch
    └── README.md
```

Only add a patch when the base kernel actually requires that change.

Do not create patches just for documentation purposes.

---

# 22. Yocto Integration

If this project is built using Yocto, kernel patches can be applied
through a custom kernel recipe.

Example:

```text
meta-stm32mp157-gpio/
└── recipes-kernel/
    └── linux/
        ├── linux-stm32mp/
        │   ├── 0001-gpio-add-virtual-gpio-driver.patch
        │   ├── 0002-arm-dts-stm32mp157-gpio-test.patch
        │   └── linux-stm32mp.bbappend
```

Example `bbappend`:

```bitbake
FILESEXTRAPATHS:prepend := "${THISDIR}/linux-stm32mp:"

SRC_URI += " \
    file://0001-gpio-add-virtual-gpio-driver.patch \
    file://0002-arm-dts-stm32mp157-gpio-test.patch \
"
```

Yocto then applies the patches during the kernel build.

---

# 23. Patch Verification

After applying patches:

```bash
git status
```

Build the kernel:

```bash
make -j$(nproc)
```

Build Device Tree:

```bash
make dtbs -j$(nproc)
```

Build modules:

```bash
make modules -j$(nproc)
```

Install/deploy according to the STM32MP1 BSP build system.

---

# 24. Testing Checklist

After deploying the modified kernel to STM32MP157-DK2:

### Kernel

```bash
uname -a
```

### GPIO subsystem

```bash
gpiodetect
```

### GPIO information

```bash
gpioinfo
```

### Kernel logs

```bash
dmesg | grep -i gpio
```

### Virtual GPIO driver

```bash
lsmod | grep virtual_gpio
```

### GPIO test

```bash
gpioset gpiochipX 0=1
gpioget gpiochipX 0
```

### Application

```bash
./led_blink
```

---

# 25. Troubleshooting

## Patch does not apply

Check:

```bash
git apply --check patch-name.patch
```

If there are conflicts:

```bash
git status
```

Review the affected files and resolve the conflict.

---

## GPIO driver does not appear

Check:

```bash
dmesg | grep virtual_gpio
```

Check:

```bash
lsmod | grep virtual_gpio
```

If necessary:

```bash
sudo insmod virtual_gpio.ko
```

---

## No gpiochip appears

Check:

```bash
gpiodetect
```

and:

```bash
ls -l /dev/gpiochip*
```

Also check:

```bash
dmesg | grep -i gpio
```

---

## Permission denied

Check:

```bash
ls -l /dev/gpiochip*
```

For development, test with:

```bash
sudo gpiodetect
sudo gpioinfo
```

Production systems should use appropriate udev/group permissions rather
than running applications permanently as root.

---

# 26. Important Difference

This project contains two different GPIO paths.

### Physical GPIO

```text
STM32MP157
    |
    v
STM32 GPIO Controller
    |
    v
Device Tree
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
LED / Button
```

### Virtual GPIO

```text
Linux Kernel
    |
    v
virtual_gpio.c
    |
    v
gpio_chip
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
Application
```

The virtual GPIO path is primarily intended for:

* Driver development
* GPIO API learning
* Application testing
* CI testing
* Hardware-independent testing
* Debugging
* Demonstrating the Linux GPIO subsystem

---

# 27. Development Recommendation

For this STM32MP157-DK2 project, develop in the following order:

```text
1. Boot STM32MP157-DK2
          |
          v
2. Verify Linux GPIO subsystem
          |
          v
3. Verify physical GPIO
          |
          v
4. Enable GPIO character device
          |
          v
5. Verify libgpiod
          |
          v
6. Implement virtual_gpio.c
          |
          v
7. Register gpio_chip
          |
          v
8. Verify gpiochip
          |
          v
9. Test gpioset/gpioget
          |
          v
10. Test C applications
          |
          v
11. Add Device Tree configuration
          |
          v
12. Convert modifications into patches
          |
          v
13. Integrate patches into Yocto
          |
          v
14. Final board validation
```

---

# 28. Final Goal

The final project should demonstrate the complete embedded Linux
GPIO stack:

```text
                 STM32MP157-DK2
                        |
                        v
                Bootloader / Linux
                        |
                        v
                  Device Tree
                        |
                        v
                Linux GPIO Core
                        |
             +----------+----------+
             |                     |
             v                     v
      STM32 GPIO Driver     Virtual GPIO Driver
             |                     |
             v                     v
       Physical GPIO          Virtual GPIO
             |                     |
             +----------+----------+
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
          +-------------+-------------+
          |             |             |
          v             v             v
        LED          Button          PWM
```

This gives the project a clear **kernel → GPIO subsystem → driver → Device Tree → libgpiod → application** architecture rather than simply being a collection of GPIO C programs.

```
```



