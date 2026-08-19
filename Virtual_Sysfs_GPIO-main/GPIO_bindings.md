# GPIO Bindings in Device Tree

## Overview

GPIO bindings define how:
```text
GPIO hardware is described in Device Tree
```

for the Linux kernel.

GPIO bindings are essential in:
- Embedded Linux
- Device Tree (DTS/DTB)
- Board Support Packages (BSP)
- Linux driver development

They allow the kernel to:
- identify GPIO controllers
- configure GPIO pins
- connect GPIOs to devices
- manage interrupts and pin states

---

# 1. What are GPIO Bindings?

GPIO bindings are:
```text
standardized Device Tree descriptions for GPIO hardware
```

They define:
- GPIO controller information
- GPIO pin usage
- GPIO relationships between devices

---

# 2. Why GPIO Bindings are Needed

Linux kernel must know:
- which GPIO controller exists
- how many GPIO pins available
- which device uses which GPIO

Without bindings:
```text
kernel cannot configure GPIO hardware properly
```

---

# 3. High-Level GPIO Binding Flow

```text
DTS GPIO Node
      ↓
Kernel Parses GPIO Bindings
      ↓
GPIO Controller Driver Loaded
      ↓
GPIO Subsystem Initialized
      ↓
Drivers Request GPIOs
``` id="flow1"

---

# 4. GPIO Architecture in Linux

```text
+--------------------------+
| User Applications        |
+--------------------------+
| Linux GPIO Subsystem     |
+--------------------------+
| GPIO Controller Driver   |
+--------------------------+
| GPIO Hardware            |
+--------------------------+
``` id="arch1"

---

# 5. GPIO Controller Node

GPIO hardware described using:
```text
Device Tree node
```

---

# 6. Basic GPIO Controller Example

```dts
gpio0: gpio@4804c000 {
    compatible = "vendor,gpio";
    reg = <0x4804c000 0x1000>;
    gpio-controller;
    #gpio-cells = <2>;
};
``` id="gpio1"

---

# 7. Important GPIO Binding Properties

| Property | Purpose |
|----------|---------|
| compatible | Driver matching |
| reg | MMIO registers |
| gpio-controller | Declares GPIO controller |
| #gpio-cells | GPIO specifier format |
| interrupts | IRQ support |

---

# 8. gpio-controller Property

This property tells kernel:
```text
this node is a GPIO controller
```

Example:

```dts
gpio-controller;
``` id="ctrl1"

---

# 9. #gpio-cells Property

Defines:
```text
number of cells used in GPIO specifier
```

Example:

```dts
#gpio-cells = <2>;
``` id="cells1"

---

# 10. GPIO Specifier

A GPIO specifier describes:
- GPIO controller
- GPIO pin number
- flags

---

# 11. GPIO Specifier Format

Typical format:

```text
<&gpio_controller pin flags>
```

---

# 12. GPIO Usage Example

```dts
reset-gpios = <&gpio0 5 GPIO_ACTIVE_LOW>;
``` id="use1"

Meaning:
- gpio controller = gpio0
- pin number = 5
- active low signal

---

# 13. GPIO Flags

Common flags:

| Flag | Meaning |
|------|---------|
| GPIO_ACTIVE_HIGH | Active high |
| GPIO_ACTIVE_LOW | Active low |

---

# 14. Active High vs Active Low

---

## Active High

```text
HIGH voltage = asserted
```

---

## Active Low

```text
LOW voltage = asserted
```

---

# 15. GPIO Controller Label

Example:

```dts
gpio0:
```

creates:
```text
phandle label
```

---

# 16. Phandle References

GPIO consumers reference controller using:
```text
&gpio0
```

---

# 17. GPIO Consumer Devices

Devices using GPIOs:
- LEDs
- buttons
- reset lines
- chip selects
- interrupts

---

# 18. GPIO Consumer Example

```dts
led {
    gpios = <&gpio0 10 GPIO_ACTIVE_HIGH>;
};
``` id="cons1"

---

# 19. GPIO Naming Convention

Property names usually end with:
```text
-gpios
```

Examples:
- reset-gpios
- enable-gpios
- irq-gpios

---

# 20. Linux GPIO Parsing Flow

```text
Kernel Parses DTS
       ↓
GPIO Controller Registered
       ↓
Consumer Requests GPIO
       ↓
GPIO Mapped
``` id="flow2"

---

# 21. GPIO Descriptor API

Modern Linux uses:
```text
GPIO descriptor API
```

instead of old integer GPIO API.

---

# 22. Driver GPIO Retrieval

Example:

```c
gpiod_get(dev, "reset", GPIOD_OUT_HIGH);
``` id="drv1"

---

# 23. Mapping to Device Tree

Driver request:

```c
gpiod_get(dev, "reset", ...)
```

matches:

```dts
reset-gpios = <...>;
``` id="map1"

---

# 24. GPIO Controller Driver Matching

Controller uses:
```text
compatible string
```

---

# 25. GPIO Compatible Example

```dts
compatible = "vendor,mygpio";
``` id="comp1"

---

# 26. GPIO Interrupt Support

GPIO pins may generate:
```text
interrupts
```

---

# 27. GPIO Interrupt Example

```dts
interrupt-parent = <&gpio0>;
interrupts = <5 IRQ_TYPE_EDGE_RISING>;
``` id="irq1"

---

# 28. GPIO and Pin Control

GPIO often works with:
```text
pinctrl subsystem
```

for pin multiplexing.

---

# 29. Pinctrl Example

```dts
pinctrl-0 = <&uart_pins>;
``` id="pin1"

---

# 30. GPIO Hogging

GPIOs automatically configured during boot.

---

# 31. GPIO Hog Example

```dts
gpio-hog;
output-high;
``` id="hog1"

---

# 32. GPIO Hog Purpose

Useful for:
- power enable
- reset lines
- default board setup

---

# 33. LED GPIO Binding Example

```dts
leds {
    compatible = "gpio-leds";

    led0 {
        gpios = <&gpio0 3 GPIO_ACTIVE_HIGH>;
    };
};
``` id="led1"

---

# 34. Button GPIO Binding Example

```dts
buttons {
    compatible = "gpio-keys";

    button0 {
        gpios = <&gpio0 7 GPIO_ACTIVE_LOW>;
    };
};
``` id="btn1"

---

# 35. SPI Chip Select GPIO

SPI devices may use:
```text
GPIO chip select
```

---

# 36. SPI GPIO Example

```dts
cs-gpios = <&gpio0 12 GPIO_ACTIVE_LOW>;
``` id="spi1"

---

# 37. Reset GPIO Example

```dts
reset-gpios = <&gpio1 2 GPIO_ACTIVE_LOW>;
``` id="rst1"

---

# 38. Power Enable GPIO

```dts
enable-gpios = <&gpio0 8 GPIO_ACTIVE_HIGH>;
``` id="pwr1"

---

# 39. GPIO Controller Internals

Controller manages:
- direction
- value
- interrupts
- debounce

---

# 40. GPIO Direction

Pin can be:
- input
- output

---

# 41. GPIO Direction APIs

```c
gpiod_direction_input();
gpiod_direction_output();
``` id="api1"

---

# 42. GPIO Value APIs

```c
gpiod_set_value();
gpiod_get_value();
``` id="api2"

---

# 43. Device Tree GPIO Workflow

```text
DTS Defines GPIO
       ↓
DTB Generated
       ↓
Kernel Parses GPIO Bindings
       ↓
GPIO Driver Loaded
       ↓
Consumers Access GPIO
``` id="flow3"

---

# 44. GPIO Numbering

Linux internally assigns:
```text
GPIO descriptors
```

instead of fixed numbers.

---

# 45. Deprecated Integer GPIO API

Old API:
```c
gpio_request();
gpio_set_value();
```

Modern kernels prefer:
```text
descriptor API
```

---

# 46. Debugging GPIOs

---

## View GPIOs

```bash
gpioinfo
``` id="dbg1"

---

## Export GPIO (Old Method)

```bash
/sys/class/gpio/
``` id="dbg2"

---

## Check Device Tree

```bash
cat /proc/device-tree/
``` id="dbg3"

---

# 47. Common GPIO Binding Problems

| Problem | Description |
|---------|-------------|
| Wrong pin number | Incorrect signal |
| Wrong polarity | Device malfunction |
| Missing pinctrl | Pin not configured |
| Wrong compatible | Driver not loaded |

---

# 48. GPIO Binding YAML Files

Modern bindings defined in:
```text
YAML schema files
```

Location:

```text
Documentation/devicetree/bindings/gpio/
```

---

# 49. GPIO Binding Validation

Kernel validates:
- property names
- cell counts
- compatible strings

---

# 50. GPIO Binding Example (Complete)

```dts
gpio0: gpio@4804c000 {
    compatible = "vendor,gpio";
    reg = <0x4804c000 0x1000>;
    gpio-controller;
    #gpio-cells = <2>;
};

device {
    reset-gpios = <&gpio0 5 GPIO_ACTIVE_LOW>;
};
``` id="full1"

---

# 51. Embedded Linux Boot Flow

```text
Bootloader Loads DTB
        ↓
Kernel Parses GPIO Nodes
        ↓
GPIO Controllers Registered
        ↓
Consumer Drivers Probe
        ↓
GPIOs Configured
``` id="boot1"

---

# 52. Advantages of GPIO Bindings

| Advantage | Description |
|-----------|-------------|
| Hardware abstraction | Yes |
| Portable kernels | Yes |
| Easy configuration | Yes |
| Reusable drivers | Yes |

---

# 53. GPIO Binding Best Practices

- use descriptive property names
- define polarity correctly
- follow YAML bindings
- use descriptor API

---

# 54. Complete GPIO Binding Workflow

```text
Board Hardware
      ↓
GPIO Defined in DTS
      ↓
DTB Generated
      ↓
Kernel Parses GPIO Bindings
      ↓
GPIO Controller Driver Loaded
      ↓
Consumer Drivers Request GPIOs
      ↓
GPIOs Used by Devices
``` id="final1"

---

# 55. Summary

- GPIO bindings describe GPIO hardware in Device Tree
- GPIO controllers use gpio-controller property
- Devices reference GPIOs using phandles
- Linux uses GPIO subsystem and descriptor API
- Essential for Embedded Linux hardware control
- GPIO bindings enable flexible hardware abstraction

---

````
