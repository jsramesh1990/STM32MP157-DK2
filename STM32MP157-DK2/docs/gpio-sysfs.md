# `STM32MP157-DK2/docs/gpio-sysfs.md`

````markdown
# GPIO Sysfs Interface - STM32MP157-DK2

## 1. Overview

This document describes the legacy Linux GPIO Sysfs interface used in
the STM32MP157-DK2 GPIO Control project.

The Sysfs interface provides GPIO control through the virtual filesystem:

    /sys/class/gpio/

The application can perform the following operations:

- Export GPIO
- Unexport GPIO
- Configure GPIO direction
- Write GPIO value
- Read GPIO value
- Toggle GPIO
- Configure input/output
- Debug GPIO state
- Validate GPIO functionality

> Note:
> GPIO Sysfs is a legacy interface and has been deprecated in modern
> Linux kernels. For new applications, the Linux GPIO character-device
> interface through libgpiod is recommended.

This project retains Sysfs support for:

- Learning Linux GPIO internals
- Legacy BSP compatibility
- Debugging
- Comparison with libgpiod
- Testing older Linux systems

---

# 2. GPIO Sysfs Architecture

The GPIO Sysfs architecture is:

```text
+-----------------------------+
|       User Application      |
|                             |
| gpio_toggle.c               |
| led_blink.c                 |
+-------------+---------------+
              |
              v
+-----------------------------+
|       GPIO Common API       |
|                             |
| gpio-common.c               |
+-------------+---------------+
              |
              v
+-----------------------------+
|      GPIO Sysfs Backend     |
|                             |
| gpio-sysfs.c                |
+-------------+---------------+
              |
              v
+-----------------------------+
|     Linux GPIO Subsystem    |
+-------------+---------------+
              |
              v
+-----------------------------+
|     STM32 GPIO Driver       |
+-------------+---------------+
              |
              v
+-----------------------------+
|       STM32MP157 GPIO       |
|          Hardware           |
+-----------------------------+
````

---

# 3. Sysfs GPIO Path

The main GPIO Sysfs directory is:

```text
/sys/class/gpio/
```

Typical contents may include:

```text
/sys/class/gpio/
├── export
├── unexport
├── gpio17/
│   ├── direction
│   ├── value
│   ├── active_low
│   └── ...
└── ...
```

The exact GPIO number depends on the Linux kernel and GPIO controller
configuration.

Do not assume that a physical STM32 pin such as PA5 automatically maps
to Linux GPIO number 5.

Always verify the mapping on the target board.

---

# 4. Basic Sysfs Operations

## 4.1 Export GPIO

To make a GPIO available through Sysfs:

```bash
echo 17 > /sys/class/gpio/export
```

After successful export:

```text
/sys/class/gpio/gpio17/
```

should appear.

Check:

```bash
ls -l /sys/class/gpio/
```

---

# 5. Configure GPIO Direction

GPIO can be configured as:

```text
input
output
```

Set GPIO17 as output:

```bash
echo out > /sys/class/gpio/gpio17/direction
```

Set GPIO17 as input:

```bash
echo in > /sys/class/gpio/gpio17/direction
```

Check current direction:

```bash
cat /sys/class/gpio/gpio17/direction
```

Expected:

```text
out
```

or:

```text
in
```

---

# 6. Write GPIO Value

For an output GPIO:

Set HIGH:

```bash
echo 1 > /sys/class/gpio/gpio17/value
```

Set LOW:

```bash
echo 0 > /sys/class/gpio/gpio17/value
```

Read current value:

```bash
cat /sys/class/gpio/gpio17/value
```

Expected:

```text
0
```

or:

```text
1
```

---

# 7. Read GPIO Input

For an input GPIO:

```bash
echo in > /sys/class/gpio/gpio18/direction
```

Read the GPIO:

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

For a push button:

```text
Button Released
       |
       v
GPIO = 1

Button Pressed
       |
       v
GPIO = 0
```

The actual logic depends on the hardware pull-up/pull-down and
active-low configuration.

---

# 8. Unexport GPIO

After finishing GPIO access:

```bash
echo 17 > /sys/class/gpio/unexport
```

This removes:

```text
/sys/class/gpio/gpio17/
```

The GPIO is then released from Sysfs ownership.

---

# 9. Complete GPIO Sysfs Flow

The standard flow is:

```text
1. Export GPIO
       |
       v
2. Configure Direction
       |
       v
3. Read / Write GPIO
       |
       v
4. Perform Application Operation
       |
       v
5. Unexport GPIO
```

Example:

```bash
echo 17 > /sys/class/gpio/export
echo out > /sys/class/gpio/gpio17/direction
echo 1 > /sys/class/gpio/gpio17/value
sleep 1
echo 0 > /sys/class/gpio/gpio17/value
echo 17 > /sys/class/gpio/unexport
```

---

# 10. Project Sysfs Implementation

The project contains:

```text
STM32MP157-DK2/
└── application/
    ├── include/
    │   └── gpio_sysfs.h
    │
    └── src/
        └── gpio-sysfs.c
```

The Sysfs backend provides functions such as:

```c
gpio_export();
gpio_unexport();
gpio_set_direction();
gpio_set_value();
gpio_get_value();
```

These functions hide the filesystem operations from the application.

---

# 11. GPIO Sysfs Header

The header file is:

```text
application/include/gpio_sysfs.h
```

A typical interface is:

```c
#ifndef GPIO_SYSFS_H
#define GPIO_SYSFS_H

int gpio_export(unsigned int gpio);
int gpio_unexport(unsigned int gpio);

int gpio_set_direction(unsigned int gpio,
                       const char *direction);

int gpio_set_value(unsigned int gpio,
                   unsigned int value);

int gpio_get_value(unsigned int gpio);

#endif
```

The exact declarations should match the implementation in
`gpio-sysfs.c`.

---

# 12. gpio_export()

The purpose of `gpio_export()` is to expose a GPIO through Sysfs.

Conceptually:

```c
int gpio_export(unsigned int gpio)
{
    int fd;

    fd = open("/sys/class/gpio/export", O_WRONLY);

    if (fd < 0)
        return -1;

    // Write GPIO number

    close(fd);

    return 0;
}
```

Flow:

```text
gpio_export(17)
       |
       v
open("/sys/class/gpio/export")
       |
       v
write("17")
       |
       v
Linux GPIO subsystem
       |
       v
/sys/class/gpio/gpio17/
```

---

# 13. gpio_unexport()

The purpose of `gpio_unexport()` is to release the GPIO.

Example concept:

```c
int gpio_unexport(unsigned int gpio)
{
    int fd;

    fd = open("/sys/class/gpio/unexport", O_WRONLY);

    if (fd < 0)
        return -1;

    // Write GPIO number

    close(fd);

    return 0;
}
```

Flow:

```text
gpio_unexport(17)
       |
       v
/sys/class/gpio/unexport
       |
       v
GPIO released
```

---

# 14. gpio_set_direction()

This function writes the direction to:

```text
/sys/class/gpio/gpioN/direction
```

Example:

```c
gpio_set_direction(17, "out");
```

Internally:

```text
GPIO17
  |
  v
/sys/class/gpio/gpio17/direction
  |
  v
write("out")
```

For input:

```c
gpio_set_direction(18, "in");
```

---

# 15. gpio_set_value()

This function writes:

```text
0
```

or:

```text
1
```

to:

```text
/sys/class/gpio/gpioN/value
```

Example:

```c
gpio_set_value(17, 1);
```

Flow:

```text
gpio_set_value()
       |
       v
open gpio17/value
       |
       v
write("1")
       |
       v
Linux GPIO subsystem
       |
       v
STM32 GPIO driver
       |
       v
Physical GPIO HIGH
```

---

# 16. gpio_get_value()

This function reads:

```text
/sys/class/gpio/gpioN/value
```

Example:

```c
int value;

value = gpio_get_value(18);
```

Flow:

```text
Physical GPIO
      |
      v
STM32 GPIO controller
      |
      v
Linux GPIO driver
      |
      v
/sys/class/gpio/gpio18/value
      |
      v
read()
      |
      v
Application
```

---

# 17. LED Control Using Sysfs

Assume GPIO17 is connected to an LED.

First export:

```bash
echo 17 > /sys/class/gpio/export
```

Configure output:

```bash
echo out > /sys/class/gpio/gpio17/direction
```

Turn LED ON:

```bash
echo 1 > /sys/class/gpio/gpio17/value
```

Turn LED OFF:

```bash
echo 0 > /sys/class/gpio/gpio17/value
```

Cleanup:

```bash
echo 17 > /sys/class/gpio/unexport
```

Flow:

```text
Application
     |
     v
GPIO Sysfs
     |
     v
Linux GPIO subsystem
     |
     v
STM32 GPIO driver
     |
     v
STM32 GPIO pin
     |
     v
LED
```

---

# 18. LED Blink Using Sysfs

Example:

```bash
#!/bin/sh

GPIO=17

echo $GPIO > /sys/class/gpio/export
echo out > /sys/class/gpio/gpio$GPIO/direction

for i in 1 2 3 4 5
do
    echo 1 > /sys/class/gpio/gpio$GPIO/value
    sleep 1

    echo 0 > /sys/class/gpio/gpio$GPIO/value
    sleep 1
done

echo $GPIO > /sys/class/gpio/unexport
```

Flow:

```text
GPIO17 HIGH
    |
    v
LED ON
    |
  1 sec
    |
    v
GPIO17 LOW
    |
    v
LED OFF
    |
  1 sec
    |
    v
Repeat
```

---

# 19. Button Input Using Sysfs

Assume GPIO18 is connected to a button.

Export:

```bash
echo 18 > /sys/class/gpio/export
```

Configure input:

```bash
echo in > /sys/class/gpio/gpio18/direction
```

Read:

```bash
cat /sys/class/gpio/gpio18/value
```

Example application:

```bash
while true
do
    value=$(cat /sys/class/gpio/gpio18/value)

    if [ "$value" = "0" ]; then
        echo "Button Pressed"
    else
        echo "Button Released"
    fi

    sleep 0.1
done
```

---

# 20. Sysfs Polling

Sysfs applications commonly use polling.

Example:

```c
while (running)
{
    value = gpio_get_value(18);

    if (value == 0)
        printf("Button pressed\n");

    usleep(100000);
}
```

Flow:

```text
+------------------+
| Read GPIO value  |
+--------+---------+
         |
         v
+------------------+
| Check GPIO state |
+--------+---------+
         |
         v
+------------------+
| Sleep 100 ms     |
+--------+---------+
         |
         +---------> Repeat
```

Advantages:

* Simple
* Easy to debug
* Easy to implement

Disadvantages:

* CPU overhead
* Polling latency
* Not ideal for high-frequency GPIO
* Less efficient than event-driven GPIO

---

# 21. GPIO Active Low

Some hardware signals are active-low.

Example:

```text
active_low = 1
```

means:

```text
GPIO = 0 -> Logical ON
GPIO = 1 -> Logical OFF
```

The Sysfs interface can expose:

```text
/sys/class/gpio/gpio17/active_low
```

Read:

```bash
cat /sys/class/gpio/gpio17/active_low
```

Set active-low:

```bash
echo 1 > /sys/class/gpio/gpio17/active_low
```

Set normal polarity:

```bash
echo 0 > /sys/class/gpio/gpio17/active_low
```

The actual use of `active_low` must match the board design.

---

# 22. GPIO Edge Detection with Sysfs

Sysfs can expose:

```text
/sys/class/gpio/gpioN/edge
```

Possible settings include:

```text
none
rising
falling
both
```

Example:

```bash
echo falling > /sys/class/gpio/gpio18/edge
```

Then an application can monitor the GPIO file descriptor using
`poll()` or `select()`.

Conceptually:

```text
Button
   |
   v
GPIO transition
   |
   v
GPIO Sysfs
   |
   v
poll()
   |
   v
Application wakes
   |
   v
Read GPIO value
```

---

# 23. Sysfs Interrupt/Event Example

A C application can conceptually use:

```c
struct pollfd pfd;

pfd.fd = fd;
pfd.events = POLLPRI | POLLERR;

poll(&pfd, 1, -1);
```

When an edge occurs:

```text
GPIO transition
       |
       v
Kernel GPIO subsystem
       |
       v
Sysfs event
       |
       v
poll()
       |
       v
Application
```

For new applications, equivalent event handling through libgpiod is
preferred.

---

# 24. Device Tree Relationship

The GPIO Sysfs interface does not directly configure the STM32 GPIO
hardware registers.

The flow is:

```text
Device Tree
    |
    v
STM32 GPIO Driver
    |
    v
Linux GPIO Subsystem
    |
    v
GPIO Sysfs
    |
    v
User Application
```

Device Tree defines:

* GPIO controller
* GPIO pinmux
* GPIO capabilities
* GPIO consumer relationships
* Pull-up/pull-down
* Drive configuration
* Active-low behavior

---

# 25. Pin Multiplexing

STM32MP157 pins support multiple alternate functions.

Example:

```text
STM32 Pin
    |
    +-- GPIO
    |
    +-- UART
    |
    +-- SPI
    |
    +-- I2C
    |
    +-- PWM
```

If the pin is configured for UART or SPI, it may not be available as a
GPIO.

Therefore, before testing Sysfs GPIO, verify pinctrl configuration.

Useful command:

```bash
cat /sys/kernel/debug/pinctrl/*/pinmux-pins
```

---

# 26. STM32MP157-DK2 GPIO Validation

On the board:

## Step 1

Check kernel:

```bash
uname -a
```

## Step 2

Check GPIO Sysfs:

```bash
ls -l /sys/class/gpio/
```

## Step 3

Check GPIO controllers:

```bash
cat /sys/kernel/debug/gpio
```

## Step 4

Find the correct GPIO number.

Do not assume that the GPIO number corresponds directly to the
physical pin number.

## Step 5

Export:

```bash
echo <GPIO_NUMBER> > /sys/class/gpio/export
```

## Step 6

Configure:

```bash
echo out > /sys/class/gpio/gpio<GPIO_NUMBER>/direction
```

## Step 7

Test:

```bash
echo 1 > /sys/class/gpio/gpio<GPIO_NUMBER>/value
```

Then:

```bash
echo 0 > /sys/class/gpio/gpio<GPIO_NUMBER>/value
```

## Step 8

Cleanup:

```bash
echo <GPIO_NUMBER> > /sys/class/gpio/unexport
```

---

# 27. Project Test Flow

The project can validate Sysfs using:

```text
application/
    |
    v
gpio-sysfs.c
    |
    +-- gpio_export()
    |
    +-- gpio_set_direction()
    |
    +-- gpio_set_value()
    |
    +-- gpio_get_value()
    |
    +-- gpio_unexport()
```

Example:

```text
Start
  |
  v
Export GPIO
  |
  v
Set Direction
  |
  v
Write GPIO
  |
  v
Read GPIO
  |
  v
Verify Result
  |
  v
Unexport GPIO
  |
  v
Exit
```

---

# 28. Example C Application

A simple Sysfs LED application:

```c
#include <stdio.h>
#include <unistd.h>

#include "gpio_sysfs.h"

#define LED_GPIO 17

int main(void)
{
    if (gpio_export(LED_GPIO) < 0) {
        perror("gpio_export");
        return 1;
    }

    if (gpio_set_direction(LED_GPIO, "out") < 0) {
        perror("gpio_set_direction");
        gpio_unexport(LED_GPIO);
        return 1;
    }

    for (int i = 0; i < 10; i++) {

        gpio_set_value(LED_GPIO, 1);
        printf("LED ON\n");
        sleep(1);

        gpio_set_value(LED_GPIO, 0);
        printf("LED OFF\n");
        sleep(1);
    }

    gpio_unexport(LED_GPIO);

    return 0;
}
```

Compile:

```bash
gcc led_blink.c gpio-sysfs.c \
    -I../include \
    -o led_blink
```

Run:

```bash
sudo ./led_blink
```

The GPIO number must be replaced with the GPIO actually connected to
the selected LED on the STM32MP157-DK2 setup.

---

# 29. Error Handling

The Sysfs implementation should check every filesystem operation.

For example:

```c
fd = open(path, O_WRONLY);

if (fd < 0) {
    perror("open");
    return -1;
}
```

Check:

```c
write(fd, ...);
```

and:

```c
close(fd);
```

Typical errors:

```text
Permission denied
Device or resource busy
No such file or directory
Invalid argument
Operation not permitted
```

---

# 30. Permission Handling

GPIO Sysfs access may require root privileges.

Example:

```bash
sudo ./gpio-sysfs
```

Check permissions:

```bash
ls -l /sys/class/gpio/
```

For production systems, avoid running the entire application as root
when possible.

Use appropriate Linux permissions, capabilities or a controlled
service configuration.

---

# 31. GPIO Already Exported

If:

```bash
echo 17 > /sys/class/gpio/export
```

returns an error because GPIO17 is already exported, check:

```bash
ls /sys/class/gpio/
```

If:

```text
gpio17
```

already exists, use it directly.

For example:

```bash
echo out > /sys/class/gpio/gpio17/direction
```

Do not blindly unexport a GPIO that is owned by another kernel driver
or application.

---

# 32. GPIO Busy Problem

A GPIO may be owned by another subsystem.

Examples:

```text
GPIO
 |
 +-- LED driver
 |
 +-- Button driver
 |
 +-- SPI
 |
 +-- I2C
 |
 +-- UART
 |
 +-- PWM
 |
 +-- Another application
```

Check:

```bash
cat /sys/kernel/debug/gpio
```

and:

```bash
gpioinfo
```

If a GPIO is already claimed, the Sysfs application should not attempt
to take control of it.

---

# 33. Debugging Commands

## Check GPIO subsystem

```bash
cat /sys/kernel/debug/gpio
```

## Check GPIO Sysfs

```bash
ls -la /sys/class/gpio/
```

## Check pinmux

```bash
cat /sys/kernel/debug/pinctrl/*/pinmux-pins
```

## Check GPIO-related kernel logs

```bash
dmesg | grep -i gpio
```

## Check GPIO character devices

```bash
ls -l /dev/gpiochip*
```

## Check GPIO lines

```bash
gpioinfo
```

---

# 34. Sysfs vs libgpiod

| Feature          | Sysfs              | libgpiod         |
| ---------------- | ------------------ | ---------------- |
| Interface        | Virtual filesystem | Character device |
| Kernel interface | Legacy             | Modern           |
| Status           | Deprecated         | Recommended      |
| GPIO access      | File operations    | GPIO API         |
| Read/write       | Yes                | Yes              |
| Direction        | Yes                | Yes              |
| Edge events      | Limited/legacy     | Yes              |
| Line ownership   | Limited            | Better           |
| Multiple GPIOs   | Less convenient    | Better           |
| Performance      | Higher overhead    | Better           |
| New applications | Not recommended    | Recommended      |
| Project support  | Yes                | Yes              |

---

# 35. Project Backend Architecture

The project supports both APIs through a common layer.

```text
                 Application
                     |
                     v
             gpio-common.c
                     |
          +----------+----------+
          |                     |
          v                     v
   gpio-libgpiod.c        gpio-sysfs.c
          |                     |
          v                     v
       libgpiod             Sysfs GPIO
          |                     |
          v                     v
    /dev/gpiochipX      /sys/class/gpio
          |                     |
          +----------+----------+
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

---

# 36. Why the Common Layer is Important

Without the common layer:

```text
led_blink.c
     |
     +---- Sysfs implementation

button.c
     |
     +---- libgpiod implementation

gpio_toggle.c
     |
     +---- Direct implementation
```

This creates duplicated code.

With the common layer:

```text
             Applications
                  |
                  v
          GPIO Common API
                  |
          +-------+-------+
          |               |
       Sysfs           libgpiod
```

Benefits:

* Cleaner application code
* Easier backend replacement
* Easier testing
* Easier maintenance
* Better portability

---

# 37. Simulator Relationship

The project also contains a GPIO simulator.

The conceptual architecture is:

```text
                  GPIO Application
                         |
                         v
                  GPIO Common API
                         |
              +----------+----------+
              |                     |
              v                     v
         Real Hardware          Simulation
              |                     |
              v                     v
       Linux GPIO Driver       Virtual GPIO
              |                     |
              v                     v
       STM32MP157 GPIO        simulator.py
```

This allows application development without continuously connecting
physical hardware.

The simulator is useful for:

* Application testing
* GPIO state testing
* Error handling
* Automated tests
* CI testing

---

# 38. Automated Testing

A test script can validate the Sysfs backend.

Example flow:

```text
Build
 |
 v
Start simulation
 |
 v
Export GPIO
 |
 v
Set direction
 |
 v
Write HIGH
 |
 v
Read value
 |
 v
Verify HIGH
 |
 v
Write LOW
 |
 v
Read value
 |
 v
Verify LOW
 |
 v
Unexport
 |
 v
PASS
```

Example test:

```bash
./test.sh --simulation
```

Hardware test:

```bash
./test.sh --hardware
```

---

# 39. Sysfs GPIO Test Cases

### Test 1 - Export

```text
Input:
GPIO = valid GPIO

Expected:
gpioN directory created
```

### Test 2 - Invalid GPIO

```text
Input:
Invalid GPIO number

Expected:
Export fails gracefully
```

### Test 3 - Output

```text
Set:
direction = out

Expected:
GPIO output configured
```

### Test 4 - HIGH

```text
Write:
1

Expected:
GPIO = HIGH
```

### Test 5 - LOW

```text
Write:
0

Expected:
GPIO = LOW
```

### Test 6 - Input

```text
direction = in

Expected:
GPIO value can be read
```

### Test 7 - Cleanup

```text
unexport

Expected:
gpioN removed
```

---

# 40. Sysfs Limitations

GPIO Sysfs has several limitations.

### 1. Deprecated

Modern Linux kernels recommend the GPIO character-device API.

### 2. File I/O Overhead

Every operation may involve:

```text
open()
write()
read()
close()
```

which adds overhead.

### 3. Limited Line Configuration

Modern GPIO requirements are better handled by libgpiod.

### 4. Ownership

The interface provides less robust GPIO line ownership compared with
the newer GPIO character-device API.

### 5. Application Complexity

Applications need to manually manage:

```text
export
direction
value
unexport
```

---

# 41. Recommended Usage in This Project

The project should use Sysfs mainly for:

```text
+-----------------------------+
| Legacy BSP validation       |
| Learning                    |
| Debugging                   |
| Compatibility testing       |
| Comparison with libgpiod    |
+-----------------------------+
```

For production GPIO applications:

```text
Application
     |
     v
libgpiod
     |
     v
/dev/gpiochipX
```

should be preferred.

---

# 42. End-to-End STM32MP157 Flow

The complete Sysfs flow is:

```text
                STM32MP157-DK2
                       |
                       v
                Physical GPIO
                       |
                       v
              STM32 GPIO Controller
                       |
                       v
              STM32 GPIO Driver
                       |
                       v
               Linux GPIO Core
                       |
                       v
              GPIO Sysfs Interface
                       |
                       v
             /sys/class/gpio/
                       |
                       v
                gpio-sysfs.c
                       |
                       v
              GPIO Common API
                       |
                       v
                 Application
```

---

# 43. Example Complete Test

Assume a valid GPIO number has been identified as `17`.

Export:

```bash
sudo sh -c 'echo 17 > /sys/class/gpio/export'
```

Check:

```bash
ls /sys/class/gpio/gpio17
```

Configure output:

```bash
sudo sh -c 'echo out > /sys/class/gpio/gpio17/direction'
```

Set HIGH:

```bash
sudo sh -c 'echo 1 > /sys/class/gpio/gpio17/value'
```

Verify:

```bash
cat /sys/class/gpio/gpio17/value
```

Expected:

```text
1
```

Set LOW:

```bash
sudo sh -c 'echo 0 > /sys/class/gpio/gpio17/value'
```

Verify:

```bash
cat /sys/class/gpio/gpio17/value
```

Expected:

```text
0
```

Cleanup:

```bash
sudo sh -c 'echo 17 > /sys/class/gpio/unexport'
```

---

# 44. Interview Explanation

A simple technical explanation of this project is:

> "In the STM32MP157-DK2 GPIO project, I implemented a Linux GPIO
> abstraction supporting both the legacy Sysfs interface and the modern
> libgpiod interface. The Sysfs backend accesses GPIOs through
> `/sys/class/gpio`, while the modern backend uses `/dev/gpiochip`
> through libgpiod. Both interfaces ultimately communicate with the
> Linux GPIO subsystem and the STM32 GPIO controller driver. Device
> Tree is used for GPIO pinmux and hardware configuration, and I
> developed applications for LED output, GPIO toggling, button input
> and GPIO edge-event handling."

---

# 45. Key Commands

```bash
# Check Sysfs GPIO
ls /sys/class/gpio/

# Export
echo 17 > /sys/class/gpio/export

# Direction
echo out > /sys/class/gpio/gpio17/direction

# HIGH
echo 1 > /sys/class/gpio/gpio17/value

# LOW
echo 0 > /sys/class/gpio/gpio17/value

# Read
cat /sys/class/gpio/gpio17/value

# Active low
cat /sys/class/gpio/gpio17/active_low

# Unexport
echo 17 > /sys/class/gpio/unexport

# Kernel GPIO debug
cat /sys/kernel/debug/gpio

# GPIO controller information
gpiodetect

# GPIO line information
gpioinfo

# Kernel GPIO messages
dmesg | grep -i gpio
```

---

# 46. Final Summary

The GPIO Sysfs implementation in the STM32MP157-DK2 project provides
a simple user-space interface for controlling GPIOs:

```text
                 User Application
                       |
                       v
                GPIO Common API
                       |
                       v
                 gpio-sysfs.c
                       |
                       v
             /sys/class/gpio/
                       |
                       v
              Linux GPIO Core
                       |
                       v
             STM32 GPIO Driver
                       |
                       v
                STM32MP157
                       |
             +---------+---------+
             |                   |
             v                   v
            LED               Button
```

The core operations are:

```text
Export
   ↓
Direction
   ↓
Read / Write
   ↓
Event / Polling
   ↓
Unexport
```

For the **STM32MP157-DK2 project**, Sysfs provides the legacy GPIO
implementation, while **libgpiod should be the primary implementation
for modern Linux GPIO applications**.

```
```

