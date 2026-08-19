README.md
# STM32MP157-DK2 GPIO Interrupt Examples


This directory contains GPIO interrupt examples for the
STM32MP157-DK2 development board.


The examples use the Linux GPIO subsystem through
the `libgpiod` userspace API.


---


## Directory


```text
interrupt/
├── button_irq.c
├── gpio_irq.c
├── Makefile
└── README.md
1. Objective

The objective of these examples is to demonstrate:

Linux GPIO input configuration
GPIO edge detection
Rising-edge events
Falling-edge events
GPIO interrupt/event handling
libgpiod userspace API
Signal handling
GPIO resource cleanup

The examples run completely from Linux userspace.

2. Hardware

Target board:

STM32MP157-DK2

Processor:

STM32MP157

CPU architecture:

ARM Cortex-A7

Linux GPIO interface:

/dev/gpiochip*
3. Software Architecture
             User Application
                    |
                    |
             button_irq.c
                    |
                    |
             libgpiod API
                    |
                    |
             /dev/gpiochip0
                    |
                    |
          Linux GPIO Subsystem
                    |
                    |
             GPIO Controller
                    |
                    |
             STM32MP157 GPIO
                    |
                    |
                 GPIO Pin
                    |
                    |
               Push Button
4. Linux GPIO Flow

The complete GPIO interrupt flow is:

Button Press
     |
     v
GPIO electrical state changes
     |
     v
STM32 GPIO Controller
     |
     v
GPIO interrupt/event
     |
     v
Linux GPIO Driver
     |
     v
GPIO character device
     |
     v
/dev/gpiochipX
     |
     v
libgpiod
     |
     v
Application
     |
     v
button_irq.c
5. Applications

Two applications are provided.

5.1 button_irq

button_irq is a button-specific example.

It detects:

Falling Edge
    |
    +---- Button Pressed


Rising Edge
    |
    +---- Button Released

Run:

sudo ./button_irq /dev/gpiochip0 14
5.2 gpio_irq

gpio_irq is a generic GPIO interrupt monitoring application.

Run:

sudo ./gpio_irq /dev/gpiochip0 14

Output example:

============================================
 STM32MP157-DK2 GPIO Interrupt Test
============================================


GPIO chip : /dev/gpiochip0
GPIO line : 14
Consumer  : stm32mp157-gpio-irq


GPIO configured successfully.
Direction : INPUT
Edge      : RISING + FALLING
Bias      : PULL-UP


Waiting for GPIO events...
Press Ctrl+C to stop.


GPIO EVENT #1
  Line      : 14
  Type      : FALLING EDGE
  Timestamp : 12345.123456789 sec
--------------------------------------------


GPIO EVENT #2
  Line      : 14
  Type      : RISING EDGE
  Timestamp : 12346.123456789 sec
--------------------------------------------
6. Prerequisites

Install the development package:

sudo apt update
sudo apt install -y libgpiod-dev

Check installation:

pkg-config --modversion libgpiod

Check GPIO devices:

ls -l /dev/gpiochip*

Example:

/dev/gpiochip0
/dev/gpiochip1
7. GPIO Information

Install GPIO tools if required:

sudo apt install -y gpiod

Detect GPIO chips:

gpiodetect

Example:

gpiochip0 [gpio-0] (16 lines)
gpiochip1 [gpio-1] (16 lines)

Display GPIO lines:

gpioinfo

Or:

gpioinfo gpiochip0
8. Build

Enter the interrupt directory:

cd STM32MP157-DK2/examples/interrupt

Build:

make

The following applications are generated:

button_irq
gpio_irq

Check:

ls -l

Expected:

button_irq
gpio_irq
button_irq.c
gpio_irq.c
Makefile
README.md
9. Run Button Interrupt Test

Run:

sudo ./button_irq /dev/gpiochip0 14

The application configures the GPIO as:

Direction : INPUT
Bias      : PULL-UP
Events    : RISING + FALLING
10. Run Generic GPIO Interrupt Test

Run:

sudo ./gpio_irq /dev/gpiochip0 14

Press the connected button.

Expected output:

GPIO EVENT #1
  Line      : 14
  Type      : FALLING EDGE

Release the button:

GPIO EVENT #2
  Line      : 14
  Type      : RISING EDGE
11. Why Pull-Up Is Used

The button can be connected as:

3.3V
 |
 |
GPIO
 |
 |
Button
 |
 |
GND

With an internal pull-up:

Button released:


GPIO = 1

When the button is pressed:

GPIO
 |
Button
 |
GND


GPIO = 0

Therefore:

1 -> 0 = Falling Edge = Button Press


0 -> 1 = Rising Edge = Button Release
12. Interrupt vs Polling

Traditional polling:

while(1)
{
    read GPIO;


    if(value_changed)
        process_event;


    sleep();
}

The CPU continuously checks the GPIO.

Interrupt/event based approach:

GPIO event
    |
    v
Kernel detects event
    |
    v
Application waits
    |
    v
Event delivered

The application does not continuously poll the GPIO.

This is more appropriate for event-driven GPIO applications.

13. libgpiod API Used

The applications use:

gpiod_chip_open()

to open the GPIO controller.

Then:

gpiod_chip_get_line()

to obtain the GPIO line.

The line is configured using:

gpiod_line_request_both_edges_events_flags()

The application waits for an event using:

gpiod_line_event_wait()

The event is read using:

gpiod_line_event_read()

Finally resources are released using:

gpiod_line_release()
gpiod_chip_close()
14. Important APIs
Open GPIO chip
chip = gpiod_chip_open("/dev/gpiochip0");
Get GPIO line
line = gpiod_chip_get_line(
    chip,
    line_offset
);
Request interrupt events
gpiod_line_request_both_edges_events_flags(
    line,
    "stm32mp157-gpio-irq",
    GPIOD_LINE_REQUEST_FLAG_BIAS_PULL_UP
);
Wait for event
gpiod_line_event_wait(
    line,
    &timeout
);
Read event
gpiod_line_event_read(
    line,
    &event
);
15. Rising Edge

A rising edge is:

0
|
|
+--------> 1

Example:

Button Released

Application receives:

GPIOD_LINE_EVENT_RISING_EDGE
16. Falling Edge

A falling edge is:

1
|
|
+--------> 0

Example:

Button Pressed

Application receives:

GPIOD_LINE_EVENT_FALLING_EDGE
17. Signal Handling

The application handles:

SIGINT
SIGTERM

For example:

Ctrl+C

generates:

SIGINT

The signal handler changes:

running = 0;

The event loop terminates and GPIO resources are released.

18. Resource Cleanup

The application must release the GPIO line:

gpiod_line_release(line);

Then close the GPIO chip:

gpiod_chip_close(chip);

This prevents GPIO resources from remaining busy.

19. Device Tree Relationship

The GPIO controller itself is normally described by
the STM32MP157 Device Tree.

Conceptually:

Device Tree
     |
     v
STM32 GPIO Controller
     |
     v
Linux GPIO Driver
     |
     v
/dev/gpiochipX
     |
     v
libgpiod
     |
     v
Application

The userspace application normally does not directly access
the STM32 GPIO registers.

20. Debugging

Check GPIO chips:

gpiodetect

Check GPIO lines:

gpioinfo

Check kernel GPIO information:

sudo cat /sys/kernel/debug/gpio

Check kernel messages:

dmesg | grep -i gpio

Check GPIO devices:

ls -l /dev/gpiochip*
21. Check Device Tree

Check GPIO-related Device Tree nodes:

grep -R gpio /proc/device-tree 2>/dev/null

Or inspect:

ls /proc/device-tree/

The actual GPIO line/chip numbering must be verified on
the running STM32MP157 Linux system.

Do not assume that a particular GPIO number such as 14
always corresponds to the same physical header pin.

22. Testing Procedure
Step 1

Boot STM32MP157-DK2 into Linux.

Step 2

Check GPIO chips:

gpiodetect
Step 3

Check GPIO lines:

gpioinfo
Step 4

Select an available GPIO input.

Step 5

Connect the push button.

Step 6

Build:

make clean
make
Step 7

Run:

sudo ./button_irq /dev/gpiochip0 <line>
Step 8

Press the button.

Expected:

FALLING EDGE
Step 9

Release the button.

Expected:

RISING EDGE
23. Clean Build
make clean

Then:

make
24. Installation

Install:

sudo make install

Applications are installed to:

/usr/local/bin/button_irq
/usr/local/bin/gpio_irq

Run:

sudo button_irq /dev/gpiochip0 14

or:

sudo gpio_irq /dev/gpiochip0 14
25. Uninstall
sudo make uninstall
26. Complete Project Flow

The interrupt example demonstrates the complete Embedded Linux
GPIO path:

                  STM32MP157-DK2
                         |
                         v
                 Physical Button
                         |
                         v
                   GPIO Pin
                         |
                         v
              STM32 GPIO Controller
                         |
                         v
                  GPIO Interrupt
                         |
                         v
                Linux GPIO Driver
                         |
                         v
                GPIO Character Device
                         |
                         v
                  /dev/gpiochipX
                         |
                         v
                      libgpiod
                         |
                         v
                  gpio_irq.c
                         |
                         v
                Event Detection
                         |
              +----------+----------+
              |                     |
              v                     v
        Rising Edge           Falling Edge
              |                     |
              v                     v
         Button Release        Button Press
27. Learning Objectives

This example helps understand:

STM32 GPIO hardware
Device Tree GPIO configuration
Linux GPIO subsystem
GPIO character devices
libgpiod
GPIO input configuration
Pull-up configuration
Rising-edge detection
Falling-edge detection
Linux GPIO events
Userspace interrupt handling
Signal handling
Resource management
Embedded Linux debugging
28. Production Considerations

For a production application:

Avoid hard-coded GPIO numbers where possible.
Identify GPIOs using stable board/device-tree descriptions.
Validate GPIO availability before requesting.
Handle GPIO contention correctly.
Implement switch debouncing.
Use proper electrical protection.
Avoid unnecessary sudo usage.
Use appropriate udev/group permissions.
Log GPIO errors.
Release GPIO resources during shutdown.
Use the libgpiod API appropriate to the installed libgpiod version.
29. Summary

The interrupt example demonstrates how a physical button
connected to the STM32MP157 GPIO reaches a Linux userspace
application.

The important path is:

Hardware
   ↓
STM32 GPIO Controller
   ↓
Linux GPIO Driver
   ↓
/dev/gpiochipX
   ↓
libgpiod
   ↓
gpio_irq.c
   ↓
GPIO Event
   ↓
Application Action

This provides a practical example of GPIO interrupt/event
handling on the STM32MP157-DK2 platform.



### Final directory


Your directory will therefore be:


```text
STM32MP157-DK2/
└── examples/
    └── interrupt/
        ├── button_irq.c
        ├── gpio_irq.c
        ├── Makefile
        └── README.md

One important point: the 14 used in the examples is only a placeholder GPIO line offset. On the actual STM32MP157-DK2, first run gpiodetect and gpioinfo and select a GPIO line that is actually available and physically connected to your button.
