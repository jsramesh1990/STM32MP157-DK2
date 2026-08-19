
# Linux GPIO Driver

# 1. Introduction

GPIO stands for:

```text
General Purpose Input Output
````

GPIO pins are programmable digital pins available on processors, microcontrollers, and SoCs.

Linux GPIO Drivers allow the kernel and user applications to:

* Control LEDs
* Read button status
* Reset peripherals
* Enable/disable hardware
* Detect interrupts
* Communicate with external devices

GPIO drivers are heavily used in:

* Embedded Linux
* Raspberry Pi
* Automotive systems
* Industrial automation
* IoT devices
* Android devices

---

# 2. What is a GPIO Driver?

A GPIO Driver is a Linux kernel driver that manages GPIO pins using the Linux GPIO subsystem.

The driver:

* Requests GPIO pins
* Configures direction
* Reads input values
* Sets output values
* Handles GPIO interrupts

GPIO drivers usually interact with hardware through:

* Device Tree
* Platform drivers
* Character devices
* Sysfs
* GPIO descriptor APIs

---

# 3. Why Do We Use GPIO Drivers?

Without GPIO drivers:

* Applications cannot safely control hardware pins
* Multiple applications may corrupt GPIO states
* Hardware abstraction becomes impossible

GPIO drivers provide:

| Feature              | Purpose                           |
| -------------------- | --------------------------------- |
| Hardware abstraction | Independent of hardware registers |
| Safe access          | Kernel-controlled GPIO management |
| Standard APIs        | Common Linux GPIO interface       |
| Portability          | Same driver works across boards   |
| Device Tree support  | Dynamic hardware configuration    |

---

# 4. Real-Time Examples

| Hardware         | GPIO Usage        |
| ---------------- | ----------------- |
| LED              | ON/OFF control    |
| Push Button      | Input detection   |
| LCD Reset Pin    | Hardware reset    |
| WiFi Module      | Enable/Disable    |
| Sensor Interrupt | Event detection   |
| Motor Driver     | Direction control |
| Buzzer           | Sound generation  |
| Camera Module    | Power/reset       |

---

# 5. GPIO Driver Architecture

```text id="v04s6v"
+-----------------------------+
| User Space Application      |
+-------------+---------------+
              |
              v
+-----------------------------+
| Character Device (/dev)     |
+-------------+---------------+
              |
              v
+-----------------------------+
| GPIO Driver                 |
|-----------------------------|
| GPIO Request                |
| GPIO Direction              |
| GPIO Read/Write             |
| GPIO Interrupt              |
+-------------+---------------+
              |
              v
+-----------------------------+
| GPIO Controller Hardware    |
+-----------------------------+
```

---

# 6. GPIO Types

## GPIO Input

Used to read signals.

Examples:

* Buttons
* Switches
* Sensors

Example:

```c id="qqmg6j"
gpiod_get_value()
```

---

## GPIO Output

Used to generate signals.

Examples:

* LEDs
* Relays
* Reset pins

Example:

```c id="m5bqel"
gpiod_set_value()
```

---

# 7. Linux GPIO Subsystem

Linux provides a GPIO framework.

Old API:

```c id="nvcf2n"
gpio_request()
gpio_direction_output()
gpio_set_value()
```

Modern API (Recommended):

```c id="mu7v7l"
devm_gpiod_get()
gpiod_set_value()
gpiod_get_value()
```

Modern GPIO Descriptor API is preferred.

---

# 8. Important Header Files

```c id="yy4h3v"
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/gpio/consumer.h>
#include <linux/of.h>
#include <linux/delay.h>
```

---

# 9. Device Tree Basics

GPIO drivers in embedded Linux usually use Device Tree.

Example:

## mydevice.dts

```dts id="w19kk6"
mygpio {
    compatible = "myvendor,mygpio";

    led-gpios = <&gpio1 20 GPIO_ACTIVE_HIGH>;

    button-gpios = <&gpio1 21 GPIO_ACTIVE_LOW>;
};
```

---

# 10. Device Tree Explanation

| Property         | Purpose            |
| ---------------- | ------------------ |
| compatible       | Matches driver     |
| led-gpios        | LED GPIO pin       |
| GPIO_ACTIVE_HIGH | Active high signal |
| GPIO_ACTIVE_LOW  | Active low signal  |

---

# 11. GPIO Driver Flow

## Step 1 – Device Tree Match

Kernel matches:

```c id="r2g1bi"
compatible = "myvendor,mygpio"
```

with driver match table.

---

## Step 2 – probe() Called

Kernel calls:

```c id="zjivq4"
probe()
```

Driver initialization begins.

---

## Step 3 – Request GPIO

```c id="s4ejl2"
devm_gpiod_get()
```

---

## Step 4 – Configure Direction

Input or output.

---

## Step 5 – Read/Write GPIO

```c id="cvb4ut"
gpiod_set_value()
gpiod_get_value()
```

---

# 12. Full GPIO Driver Example

## gpio_driver.c

```c id="evs35y"
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/gpio/consumer.h>
#include <linux/of.h>
#include <linux/delay.h>

static struct gpio_desc *led_gpio;

static int gpio_probe(struct platform_device *pdev)
{
    printk(KERN_INFO "GPIO Driver Probe Called\n");

    /* Get GPIO from Device Tree */
    led_gpio = devm_gpiod_get(&pdev->dev,
                              "led",
                              GPIOD_OUT_LOW);

    if (IS_ERR(led_gpio)) {
        printk(KERN_ERR "Failed to get GPIO\n");
        return PTR_ERR(led_gpio);
    }

    printk(KERN_INFO "GPIO Obtained Successfully\n");

    /* Blink LED */
    gpiod_set_value(led_gpio, 1);
    msleep(1000);

    gpiod_set_value(led_gpio, 0);
    msleep(1000);

    return 0;
}

static int gpio_remove(struct platform_device *pdev)
{
    printk(KERN_INFO "GPIO Driver Removed\n");
    return 0;
}

static const struct of_device_id gpio_of_match[] = {
    { .compatible = "myvendor,mygpio" },
    { }
};

MODULE_DEVICE_TABLE(of, gpio_of_match);

static struct platform_driver gpio_driver = {
    .probe  = gpio_probe,
    .remove = gpio_remove,
    .driver = {
        .name = "my_gpio_driver",
        .of_match_table = gpio_of_match,
    },
};

module_platform_driver(gpio_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Simple GPIO Driver");
```

---

# 13. Makefile

```Makefile id="3gk38g"
obj-m += gpio_driver.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD  := $(shell pwd)

all:
	make -C $(KDIR) M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean
```

---

# 14. Compile Driver

```bash id="pqug6m"
make
```

Output:

```bash id="qf97bd"
gpio_driver.ko
```

---

# 15. Insert Driver

```bash id="8b1e17"
sudo insmod gpio_driver.ko
```

---

# 16. Check Kernel Logs

```bash id="rdd2qb"
dmesg | tail
```

Expected:

```text id="9grn9c"
GPIO Driver Probe Called
GPIO Obtained Successfully
```

---

# 17. GPIO APIs Explained

## devm_gpiod_get()

Requests GPIO from Device Tree.

Example:

```c id="blm7r3"
devm_gpiod_get(dev, "led", GPIOD_OUT_LOW);
```

Advantages:

* Automatic cleanup
* Safer resource management

---

## gpiod_set_value()

Sets GPIO output value.

Example:

```c id="2gwqfr"
gpiod_set_value(led_gpio, 1);
```

---

## gpiod_get_value()

Reads GPIO input value.

Example:

```c id="3x44if"
value = gpiod_get_value(button_gpio);
```

---

# 18. GPIO Interrupt Example

GPIOs can generate interrupts.

Used for:

* Buttons
* Motion sensors
* Touch sensors

Example flow:

```text id="fj7b5n"
GPIO Event
    ↓
IRQ Trigger
    ↓
Interrupt Handler
    ↓
Process Event
```

---

# 19. Example GPIO Interrupt APIs

```c id="f1b4gj"
gpiod_to_irq()
request_irq()
free_irq()
```

---

# 20. GPIO Driver Advantages

| Advantage           | Description                    |
| ------------------- | ------------------------------ |
| Simple              | Easy hardware control          |
| Portable            | Works across boards            |
| Device Tree Support | Dynamic configuration          |
| Safe APIs           | Kernel-managed access          |
| Flexible            | Input/output/interrupt support |
| Low Cost            | Minimal hardware requirements  |

---

# 21. GPIO Driver Disadvantages

| Disadvantage                | Description                          |
| --------------------------- | ------------------------------------ |
| Slow for High-Speed Signals | GPIO not suitable for fast protocols |
| CPU Overhead                | Polling consumes CPU                 |
| Interrupt Complexity        | Requires synchronization             |
| Hardware Dependency         | GPIO numbering differs               |
| Timing Limitations          | Not real-time precise                |

---

# 22. GPIO Driver vs Direct Register Access

| Feature         | GPIO API | Direct Register Access |
| --------------- | -------- | ---------------------- |
| Portability     | High     | Low                    |
| Safety          | High     | Risky                  |
| Maintainability | Easy     | Difficult              |
| Performance     | Moderate | Fast                   |
| Recommended     | Yes      | No                     |

---

# 23. Common Interview Questions

## Q1. What is GPIO?

GPIO stands for General Purpose Input Output.

Programmable digital pins used for hardware communication.

---

## Q2. Why Use GPIO Descriptor API?

Modern Linux kernels recommend descriptor APIs because they:

* Are safer
* Support Device Tree
* Provide automatic cleanup

---

## Q3. Difference Between GPIO Input and Output?

| Input           | Output            |
| --------------- | ----------------- |
| Reads signals   | Generates signals |
| Example: Button | Example: LED      |

---

## Q4. Why Use Device Tree?

Device Tree avoids hardcoded GPIO numbers.

Makes drivers portable across boards.

---

## Q5. What is probe()?

probe() is called when the kernel matches hardware with the driver.

Used for:

* GPIO allocation
* Hardware initialization
* Interrupt setup

---

# 24. Common Errors

## Error: Failed to get GPIO

Cause:

* Wrong Device Tree property
* GPIO already in use

Fix:

* Verify DTS file
* Check GPIO controller

---

## Error: Invalid GPIO

Cause:

* Wrong GPIO number

Fix:

* Verify board schematic
* Verify Device Tree mapping

---

## Error: Permission Denied

Cause:

* GPIO reserved by another driver

Fix:

* Release GPIO properly

---

# 25. GPIO Debugging Techniques

## View GPIO Information

```bash id="4o8w2m"
cat /sys/kernel/debug/gpio
```

---

## Check Device Tree

```bash id="p5x95n"
ls /proc/device-tree/
```

---

## Kernel Logs

```bash id="q4hmz4"
dmesg | tail
```

---

# 26. Advanced GPIO Topics

After learning basic GPIO drivers, move to:

* GPIO interrupts
* Debouncing
* Pinctrl subsystem
* PWM drivers
* SPI drivers
* I2C drivers
* Power management
* Wakeup GPIOs
* GPIO expanders
* Threaded IRQs

---

# 27. Best Practices

## Use Descriptor APIs

Preferred:

```c id="l7e8sx"
devm_gpiod_get()
```

Avoid old integer GPIO APIs.

---

## Avoid Busy Waiting

Bad:

```c id="uxfz1y"
while(1);
```

Use:

```c id="g20o2r"
msleep()
wait_event()
interrupts
```

---

## Always Handle Errors

```c id="9r36jt"
if (IS_ERR(gpio))
    return PTR_ERR(gpio);
```

---

## Use Device Tree

Avoid hardcoded GPIO numbers.

---

# 28. Real Hardware Platforms

GPIO drivers are widely used on:

* Raspberry Pi 5
* BeagleBone Black
* NVIDIA Jetson Nano
* STM32MP157

---

# 29. Summary

GPIO Drivers are one of the most important parts of embedded Linux development.

They allow Linux systems to interact with external hardware safely and efficiently.

GPIO drivers are:

* Simple
* Powerful
* Flexible
* Portable

Mastering GPIO drivers is essential before learning:

* Interrupt handling
* I2C
* SPI
* Platform drivers
* Device Tree
* Embedded Linux BSP development

---

