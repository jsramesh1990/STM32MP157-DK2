README.md
# STM32MP157-DK2 PWM Examples


This directory contains PWM examples for the
STM32MP157-DK2 development board.


The examples demonstrate how Linux userspace can control
hardware PWM exposed by the Linux kernel.


---


## Directory Structure


```text
STM32MP157-DK2/
└── examples/
    └── pwm/
        ├── Makefile
        ├── pwm_fade.c
        ├── pwm_led.c
        └── README.md
1. Objective

The objective of these examples is to demonstrate:

Linux PWM subsystem
Hardware PWM generation
PWM period
PWM duty cycle
PWM enable/disable
LED brightness control
Fade-in
Fade-out
Linux PWM sysfs interface
Device Tree PWM configuration
2. STM32MP157-DK2 PWM Architecture

The complete flow is:

                  STM32MP157-DK2
                         |
                         v
                  STM32 PWM Hardware
                         |
                         v
                  PWM Controller
                         |
                         v
                   Device Tree
                         |
                         v
                   Linux PWM Driver
                         |
                         v
                /sys/class/pwm/
                         |
                         v
                  PWM Userspace
                         |
              +----------+----------+
              |                     |
              v                     v
          pwm_led.c            pwm_fade.c
              |                     |
              +----------+----------+
                         |
                         v
                     PWM Signal
                         |
                         v
                       LED
3. PWM Concept

PWM means:

Pulse Width Modulation

A PWM signal contains:

HIGH       LOW
 |          |
 v          v


 ____       ______
|    |     |
|    |_____|
|
+------------------> Time

PWM has two important parameters:

Period
Duty Cycle
4. PWM Period

Period is the total time of one PWM cycle.

Example:

Period = 1 ms

Frequency:

Frequency = 1 / Period

Therefore:

1 / 0.001 = 1000 Hz

So:

Period = 1 ms
Frequency = 1 kHz
5. PWM Duty Cycle

Duty cycle determines how long the signal stays HIGH.

Formula:

Duty Cycle (%) =
(Duty Time / Period) × 100

Example:

Period = 1 ms
Duty   = 0.5 ms

Therefore:

Duty Cycle = 50%

Signal:

     0.5ms       0.5ms


      HIGH        LOW
       |           |
       v           v


       ____        ____
      |    |      |    |
______|    |______|    |______
6. Linux PWM Interface

On systems using the Linux PWM sysfs interface, PWM controllers
appear under:

/sys/class/pwm/

Example:

/sys/class/pwm/pwmchip0

After exporting a channel:

/sys/class/pwm/pwmchip0/pwm0/

Typical files:

period
duty_cycle
enable
polarity
7. PWM Export

A PWM channel can be exported using:

echo 0 | sudo tee /sys/class/pwm/pwmchip0/export

Then:

ls /sys/class/pwm/pwmchip0/

Expected:

device
export
npwm
power
pwm0
subsystem
uevent
unexport
8. Configure Period

For a 1 ms period:

echo 1000000 | sudo tee \
/sys/class/pwm/pwmchip0/pwm0/period

The value is in nanoseconds.

1000000 ns
    |
    v
1 ms
9. Configure Duty Cycle

For 50% duty cycle:

echo 500000 | sudo tee \
/sys/class/pwm/pwmchip0/pwm0/duty_cycle

Therefore:

Period = 1000000 ns
Duty   = 500000 ns

Duty cycle:

50%
10. Enable PWM

Enable:

echo 1 | sudo tee \
/sys/class/pwm/pwmchip0/pwm0/enable

Disable:

echo 0 | sudo tee \
/sys/class/pwm/pwmchip0/pwm0/enable
11. pwm_led

pwm_led is a simple PWM control application.

Usage:

sudo ./pwm_led <chip> <channel> <period_ns> <duty_ns>

Example:

sudo ./pwm_led 0 0 1000000 500000

This generates:

PWM chip   = 0
PWM channel = 0
Period      = 1 ms
Duty        = 0.5 ms
Duty cycle  = 50%
Frequency   = 1 kHz
12. pwm_fade

pwm_fade demonstrates changing the duty cycle dynamically.

Usage:

sudo ./pwm_fade <chip> <channel>

Example:

sudo ./pwm_fade 0 0

The application performs:

0%
 |
 v
10%
 |
 v
20%
 |
 v
30%
 |
 v
...
 |
 v
100%
 |
 v
90%
 |
 v
80%
 |
 v
...
 |
 v
0%

This produces an LED fade effect.

13. Fade-In

During fade-in:

Duty Cycle
    |
100%|                         *
    |                      *
    |                   *
    |                *
    |             *
    |          *
    |       *
    |    *
  0%| *
    +----------------------------> Time

The duty cycle continuously increases.

Therefore LED brightness increases.

14. Fade-Out

During fade-out:

Duty Cycle
    |
100%| *
    |    *
    |       *
    |          *
    |             *
    |                *
    |                   *
    |                      *
  0%|                         *
    +----------------------------> Time

The duty cycle decreases.

Therefore LED brightness decreases.

15. Build

Enter the directory:

cd STM32MP157-DK2/examples/pwm

Build:

make

Expected binaries:

pwm_led
pwm_fade

Check:

ls -l
16. Clean
make clean

Rebuild:

make clean
make
17. Check PWM Controllers

Before running the applications:

ls -l /sys/class/pwm/

Check PWM chips:

ls /sys/class/pwm/pwmchip*

Example:

/sys/class/pwm/pwmchip0

Check number of channels:

cat /sys/class/pwm/pwmchip0/npwm
18. Check PWM Configuration

Inspect:

ls /sys/class/pwm/pwmchip0/

Check device information:

readlink -f \
/sys/class/pwm/pwmchip0/device

Check kernel messages:

dmesg | grep -i pwm
19. Device Tree

PWM hardware must be enabled through the Device Tree.

Conceptually:

Device Tree
     |
     v
PWM Controller Node
     |
     v
PWM Driver
     |
     v
Linux PWM Framework
     |
     v
/sys/class/pwm/pwmchipX
     |
     v
Userspace Application

The exact PWM controller/channel and pinmux must match the
STM32MP157-DK2 hardware configuration.

20. Pin Multiplexing

The STM32MP157 GPIO pins are multiplexed.

A physical pin may support:

GPIO
UART
SPI
I2C
PWM
...

Therefore PWM requires the correct pinmux.

Conceptually:

GPIO Pin
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

Device Tree selects the required alternate function.

21. PWM LED Hardware

A typical connection is:

STM32MP157 PWM PIN
        |
        |
      220Ω
        |
        |
       LED
        |
        |
       GND

The PWM signal controls the average LED current.

Higher duty cycle:

Higher brightness

Lower duty cycle:

Lower brightness
22. PWM Frequency

For LED applications, a PWM frequency such as:

500 Hz
1 kHz
2 kHz

is commonly sufficient.

For this example:

Period = 1,000,000 ns

Therefore:

Period = 1 ms
Frequency = 1 kHz
23. Complete PWM Flow
Application
    |
    v
pwm_led.c / pwm_fade.c
    |
    v
/sys/class/pwm/
    |
    v
Linux PWM Framework
    |
    v
STM32 PWM Driver
    |
    v
STM32 PWM Controller
    |
    v
PWM Hardware
    |
    v
Pin Multiplexer
    |
    v
Physical PWM Pin
    |
    v
LED
24. PWM vs GPIO

GPIO:

HIGH
LOW

PWM:

HIGH + LOW
     |
     v
Variable duty cycle

GPIO can turn an LED:

ON
OFF

PWM can control:

Brightness

Example:

GPIO:


0%   -> OFF
100% -> ON

PWM:

10%  -> Very dim
25%  -> Low brightness
50%  -> Medium brightness
75%  -> High brightness
100% -> Full brightness
25. Debugging

Check PWM subsystem:

ls /sys/class/pwm/

Check PWM channels:

cat /sys/class/pwm/pwmchip0/npwm

Check exported channels:

ls /sys/class/pwm/pwmchip0/

Check current period:

cat /sys/class/pwm/pwmchip0/pwm0/period

Check duty:

cat /sys/class/pwm/pwmchip0/pwm0/duty_cycle

Check enable:

cat /sys/class/pwm/pwmchip0/pwm0/enable

Check kernel logs:

dmesg | grep -i pwm
26. Common Problems
PWM chip not present

Check:

ls /sys/class/pwm/

If no PWM controller appears, check:

Device Tree
PWM driver
Kernel configuration
Hardware configuration
Cannot export channel

Check:

cat /sys/class/pwm/pwmchip0/npwm

Make sure the channel exists.

Also check whether the channel is already exported.

Cannot change period

Make sure PWM is disabled before changing period:

echo 0 | sudo tee \
/sys/class/pwm/pwmchip0/pwm0/enable

Then configure:

echo 1000000 | sudo tee \
/sys/class/pwm/pwmchip0/pwm0/period
LED does not change brightness

Check:

PWM pinmux
Device Tree
PWM controller
PWM channel
LED polarity
Resistor
Physical connection
Period
Duty cycle
27. Testing Procedure
Step 1

Boot Linux on STM32MP157-DK2.

Step 2

Check PWM:

ls /sys/class/pwm/
Step 3

Check channels:

cat /sys/class/pwm/pwmchip0/npwm
Step 4

Build:

make clean
make
Step 5

Run basic PWM:

sudo ./pwm_led 0 0 1000000 500000
Step 6

Observe the LED.

Step 7

Run fade:

sudo ./pwm_fade 0 0
Step 8

Observe:

Brightness increases
        |
        v
100%
        |
        v
Brightness decreases
        |
        v
0%
28. Learning Objectives

This example demonstrates:

STM32 PWM hardware
PWM Device Tree configuration
Linux PWM driver
Linux PWM framework
PWM sysfs interface
PWM period
PWM frequency
PWM duty cycle
PWM enable/disable
Pin multiplexing
LED brightness control
Hardware PWM
Userspace PWM control
Dynamic duty-cycle modification
Embedded Linux debugging
29. Important Note

The PWM chip/channel values used in the examples:

chip    = 0
channel = 0

are examples.

The actual STM32MP157-DK2 system must be checked with:

ls /sys/class/pwm/

and:

cat /sys/class/pwm/pwmchip*/npwm

The correct PWM channel and physical pin must match the
Device Tree and board hardware configuration.

30. Summary

The PWM examples demonstrate the complete Linux PWM path:

STM32MP157 Hardware
        |
        v
Device Tree
        |
        v
Pin Multiplexing
        |
        v
STM32 PWM Controller
        |
        v
Linux PWM Driver
        |
        v
Linux PWM Framework
        |
        v
/sys/class/pwm/
        |
        v
pwm_led.c / pwm_fade.c
        |
        v
PWM Signal
        |
        v
LED Brightness

The two applications provide:

pwm_led.c
    |
    +---- Fixed PWM duty cycle


pwm_fade.c
    |
    +---- Dynamic PWM duty cycle
    |
    +---- Fade IN
    |
    +---- Fade OUT

This makes the pwm directory a practical hardware-PWM
demonstration for the STM32MP157-DK2 Embedded Linux project.



### Final structure


Your complete `examples` area will now look like:


```text
STM32MP157-DK2/
└── examples/
    ├── button/
    │   ├── button.c
    │   ├── Makefile
    │   └── README.md
    │
    ├── interrupt/
    │   ├── button_irq.c
    │   ├── gpio_irq.c
    │   ├── Makefile
    │   └── README.md
    │
    ├── led/
    │   ├── led_blink.c
    │   ├── led_control.c
    │   ├── Makefile
    │   └── README.md
    │
    └── pwm/
        ├── Makefile
        ├── pwm_fade.c
        ├── pwm_led.c
        └── README.md

Note: For a modern STM32MP157 Linux BSP, PWM may be exposed through newer kernel interfaces rather than the legacy /sys/class/pwm interface, depending on your kernel/BSP configuration. Before testing, verify what your actual STM32MP157-DK2 image exposes under /sys/class/pwm/.
