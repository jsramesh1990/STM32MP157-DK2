For the **STM32MP157-DK2** project, I recommend keeping `examples/` as a collection of **ready-to-run functional demonstrations**, separate from `application/` where the reusable GPIO library/API lives.

Use this structure:

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
    │   ├── gpio_toggle.c
    │   ├── Makefile
    │   └── README.md
    │
    └── pwm/
        ├── pwm_led.c
        ├── pwm_fade.c
        ├── Makefile
        └── README.md
```

### 1. `examples/button/`

Purpose: **GPIO input testing**

```text
button/
├── button.c
├── Makefile
└── README.md
```

Flow:

```text
Physical Button
      ↓
STM32MP157 GPIO
      ↓
Linux GPIO Controller
      ↓
libgpiod
      ↓
button.c
      ↓
"BUTTON PRESSED"
"BUTTON RELEASED"
```

`button.c` should demonstrate:

* GPIO input configuration
* GPIO value reading
* Pull-up/pull-down
* Button press detection
* Debouncing

---

### 2. `examples/interrupt/`

Purpose: **GPIO interrupt / edge detection**

```text
interrupt/
├── button_irq.c
├── gpio_irq.c
├── Makefile
└── README.md
```

Flow:

```text
Button
   ↓
GPIO Edge
   ↓
GPIO Controller
   ↓
Linux IRQ subsystem
   ↓
GPIO driver
   ↓
libgpiod event
   ↓
button_irq.c
   ↓
Application callback/event
```

You can demonstrate:

```text
Rising Edge
Falling Edge
Both Edges
Interrupt Event
```

For example:

```text
Button pressed
      ↓
Falling Edge
      ↓
GPIO interrupt
      ↓
Linux IRQ
      ↓
Userspace event
      ↓
button_irq
      ↓
"Button Pressed"
```

---

### 3. `examples/led/`

Purpose: **GPIO output**

```text
led/
├── led_blink.c
├── gpio_toggle.c
├── Makefile
└── README.md
```

`led_blink.c`:

```text
GPIO OUTPUT
     ↓
1 → LED ON
0 → LED OFF
     ↓
500 ms delay
     ↓
Repeat
```

`gpio_toggle.c`:

```text
Current GPIO
     ↓
Read
     ↓
Invert
     ↓
Write
     ↓
Repeat
```

Example:

```text
GPIO = 0
   ↓
GPIO = 1
   ↓
GPIO = 0
   ↓
GPIO = 1
```

---

### 4. `examples/pwm/`

Purpose: **PWM hardware control**

```text
pwm/
├── pwm_led.c
├── pwm_fade.c
├── Makefile
└── README.md
```

Flow:

```text
Application
    ↓
Linux PWM Framework
    ↓
STM32 PWM Driver
    ↓
STM32 Timer/PWM Hardware
    ↓
PWM Signal
    ↓
LED
```

Example:

```text
Duty Cycle

0%   → LED OFF
25%  → Dim
50%  → Medium
75%  → Bright
100% → Full ON
```

### Final `examples` directory

I would therefore use:

```text
examples/
│
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
│   ├── gpio_toggle.c
│   ├── Makefile
│   └── README.md
│
└── pwm/
    ├── pwm_led.c
    ├── pwm_fade.c
    ├── Makefile
    └── README.md
```

And importantly, **don't duplicate the GPIO implementation inside these examples**.

The examples should use your reusable application library:

```text
examples/
     │
     ├── button/button.c
     ├── interrupt/button_irq.c
     ├── led/led_blink.c
     └── pwm/pwm_led.c
              │
              ▼
application/include/
     │
     ├── gpio_common.h
     ├── gpio_libgpiod.h
     └── gpio_sysfs.h
              │
              ▼
application/src/
     │
     ├── gpio-common.c
     ├── gpio-libgpiod.c
     └── gpio-sysfs.c
              │
              ▼
       Linux GPIO subsystem
              │
              ▼
        STM32MP157-DK2
```

This gives the project a clean **library → example → hardware** architecture and is much better for explaining the project in an interview.

