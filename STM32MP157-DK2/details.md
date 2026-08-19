Yes — **STM32MP157-DK2 is a very good choice** for this project.

![Image](https://images.openai.com/static-rsc-4/odTntFlAkm0QJVi6uJbHGrtGeB69quxudmAjx4Q8tTE5NYgOq7OoWactiqFvNxj5-2Ec_91ipt5narUOmj9lnGIV6omlf6iBVhXN-dDgvOSlk8zp6Vu4RoRMHXs1zPJzj_AC9X9DZCmzps8CyCDd6rjexltNcbwX4iYTSapZe4-zX3xXQlQj0pR7bePufDLW?purpose=fullsize)

![Image](https://images.openai.com/static-rsc-4/cKgf-_gxhXCHKFQT3e3fclgDotz2htGhBtJidDh60JcnH67D2zCcRex-oaOVBh48oLaCVrtzvLmEC0tsFd943sLKwLP49BnL300w_FirK8ljwe6I2e1y6ArnoamCGiKEkGxGvtTEOhZ4h_lPoi8q2ZMj0JUf_ICiGcahobtELGBimykUSp-uECLPRDaDTQRK?purpose=fullsize)

![Image](https://images.openai.com/static-rsc-4/O6g8E17gIcUdLN67DKiGduUQ5UllBJr9e2qghHMdrjdX8w-DsNpmcGipir9obIfZOdfiMP4neP4JOw0iMigLv7IRo0u8ILf_Hr8rA5qbjvrOvAhrsn44DbKkfi-0RJ2iP-h2noWnvE9Wxx27MBpH0AszbFM-J8GuclEqwcjQ4rFR5UDYLKWIsKWX4Yq5fHqF?purpose=fullsize)

![Image](https://images.openai.com/static-rsc-4/TIJD3L2ES74UZrzJ7kqRmLVaZEqKUWcPu7GL-4Ya2pXz6ARW0RasdmYqX_Dn2cpYJG6JYG6WJw4OlhXMt6So-Zzv9-tj4Jdbi9bUp0NxOUBqhbtTmQyiGDvlOmsWJLS28Zj5ONGQqUwzbpf_55I_mmlDa8j6tRJyAuq2-L9J3asjQcbYC00Jj7M-DQ1lf19Q?purpose=fullsize)

![Image](https://images.openai.com/static-rsc-4/BHK2jIIH-Rm8_ShkakGgjvvgzzu3nWzp_j_skVmYFFjgcBCXUE7HEnl59DFny0WqzVgo_0Tr9UfjwkZnJaVYWtek29f4t0ajlsoMsBIZff91uxztbe4wmOwHeRj665A9huwjM80kALTH7SGXbrGU9ug-Wx5XSyPpL5bJAU82iUJSMPBVivbUvdsBhBGUUW_b?purpose=fullsize)

For your **Virtual Sysfs GPIO / GPIO Control System**, I would structure the project around **STM32MP157-DK2** rather than the BeagleBone Black.

### Recommended project platform

**Board:** STM32MP157-DK2
**SoC:** STM32MP157
**Application processor:** Dual Cortex-A7
**Real-time processor:** Cortex-M4
**OS:** Linux on Cortex-A7
**BSP:** OpenSTLinux / Yocto
**GPIO interface:** Linux GPIO subsystem + libgpiod
**Legacy interface:** Sysfs GPIO where supported
**Hardware:** LEDs, push buttons, GPIO interrupts

### Your project flow

```text
                    STM32MP157-DK2
                           │
                    STM32MP157 SoC
                           │
                    ┌──────┴──────┐
                    │             │
                Cortex-A7      Cortex-M4
                    │
                  Linux
                    │
              Device Tree
                    │
             GPIO Controller
                    │
          Linux GPIO Subsystem
                    │
          ┌─────────┴─────────┐
          │                   │
      libgpiod            Sysfs GPIO
          │                   │
          └─────────┬─────────┘
                    │
              Your C Program
                    │
          ┌─────────┴─────────┐
          │                   │
         LED                Button
                              │
                         GPIO Interrupt
```

### Why this board fits your repository

Your existing repository already has:

```text
gpio-sysfs.c
gpio-libgpiod.c
simulator.py
test.sh
Makefile
GPIO_bindings.md
gpio-driver.md
sysfs-example.md
```

You can extend it with:

```text
Virtual_Sysfs_GPIO/
│
├── application/
│   ├── gpio-sysfs.c
│   └── gpio-libgpiod.c
│
├── simulator/
│   └── simulator.py
│
├── kernel/
│   └── gpio-driver/
│
├── device-tree/
│   └── stm32mp157-gpio-test.dts
│
├── yocto/
│   └── meta-virtual-gpio/
│
├── scripts/
│   ├── build.sh
│   ├── deploy.sh
│   ├── test.sh
│   └── clean.sh
│
├── Makefile
└── README.md
```

### The important learning flow

You can demonstrate the complete embedded Linux GPIO path:

```text
Power ON
   ↓
TF-A / BootROM
   ↓
U-Boot
   ↓
Linux Kernel
   ↓
Device Tree
   ↓
STM32 GPIO Controller Driver
   ↓
Linux GPIO Framework
   ↓
/dev/gpiochipX
   ↓
libgpiod
   ↓
Your Application
   ↓
Physical GPIO
   ↓
LED / Button
```

And then add the **kernel-driver side**:

```text
Device Tree
     ↓
Platform Device
     ↓
GPIO Controller Driver
     ↓
GPIO Framework
     ↓
GPIO Descriptor
     ↓
GPIO Consumer Driver
     ↓
Userspace
```

### One important correction to your existing README

Your README currently presents **sysfs GPIO as if it is the preferred Linux interface**.

For a new STM32MP157 project, I would change the emphasis to:

```text
                 GPIO USERSPACE
                       │
                       ▼
                  libgpiod
                       │
                       ▼
             /dev/gpiochipN
                       │
                       ▼
              Linux GPIO Core
                       │
                       ▼
             STM32 GPIO Driver
                       │
                       ▼
                STM32MP157
```

and document **Sysfs GPIO as the legacy interface**.

That will make the repository technically stronger and more relevant to **modern Linux kernel/BSP development**.

**So yes: `STM32MP157-DK2` is a strong choice for this project, especially if your goal is to showcase Linux GPIO, Device Tree, kernel drivers, Yocto/OpenSTLinux, interrupts, and BSP development.**




Yes. With **STM32MP157-DK2**, I would **restructure your existing `Virtual_Sysfs_GPIO` repository into a real Embedded Linux GPIO/BSP project**, rather than simply changing the board name.

The important point is: your current repository is mostly a **userspace GPIO simulator/application project**. To make it a strong STM32MP157-DK2 project, we should add the complete layers:

**Hardware → Bootloader → Linux Kernel → Device Tree → GPIO controller → libgpiod → userspace → simulator → tests → Yocto/OpenSTLinux**

![Image](https://images.openai.com/static-rsc-4/odTntFlAkm0QJVi6uJbHGrtGeB69quxudmAjx4Q8tTE5NYgOq7OoWactiqFvNxj5-2Ec_91ipt5narUOmj9lnGIV6omlf6iBVhXN-dDgvOSlk8zp6Vu4RoRMHXs1zPJzj_AC9X9DZCmzps8CyCDd6rjexltNcbwX4iYTSapZe4-zX3xXQlQj0pR7bePufDLW?purpose=fullsize)

![Image](https://images.openai.com/static-rsc-4/cKgf-_gxhXCHKFQT3e3fclgDotz2htGhBtJidDh60JcnH67D2zCcRex-oaOVBh48oLaCVrtzvLmEC0tsFd943sLKwLP49BnL300w_FirK8ljwe6I2e1y6ArnoamCGiKEkGxGvtTEOhZ4h_lPoi8q2ZMj0JUf_ICiGcahobtELGBimykUSp-uECLPRDaDTQRK?purpose=fullsize)

![Image](https://images.openai.com/static-rsc-4/ZZzpdPZdHFHmrNCU5ZviStjeG2d6f95mzk1oBftjTSG19_djwtT-X3LI3UEgqIh3aFaLoE_2Tpuaj3okOgl5VBJsFPLHC23WiX3MPCoHexkqb_vK7qkcsLsnG3FBeBEOKi9WX2G0x8ut81SnwArIDcN3zon_iZwY_GcnVse2fkdMWwNUj6eRvbb3Q9v8MgTf?purpose=fullsize)

![Image](https://images.openai.com/static-rsc-4/pT-x2S_SzXQWuNlpFb8BCSNFO1R_L6DtNeog0DPZNZ8geHb3xPOANzfvArPXIT3UzXshkYWEE6rdfKqYPV65UvUBTQfIoMqzFw-CE8XcqE0XApGQwyr8eK_D7-LxMgQnp5u_PcqAKRLyfXfr2cEiM-zk8u5-2RgZwzzw1oLaspKlX0lanjhRQ0UuBuBFvhAS?purpose=fullsize)

![Image](https://images.openai.com/static-rsc-4/wBe_h8dCGPsNKkhHrVWNt4ovY-IGg6JJgYm2WI4za3d9qBm_Mt20LKSkwWt75OOXlVSk8UFGtIbyswdO28dVmedSdaGJsEZ7gMFh-hjJnye3K19IxUzfsjLIoXtWyrTU_Egwlk-f1vxrzdsMlUDCW8YGeww7-PugRFlEqtp78WZdq99JSpuSslDxOLQQwr3I?purpose=fullsize)

![Image](https://images.openai.com/static-rsc-4/SCwaq2CWTKJ7Bd7PxugSTvywxVsu2_eleyGA0ZYHjg9ainNJibu-kfxXTsRl2DhfLM8CL7iY4ILroTbHgEYkW1GCeMxH8zDEP4LVZBFn1xWa12bqi2V4XTPwwSQVB0ZmqOG71Y5NmF1ZaXq7hwNT2rU3PQDFBP4l0nntbuJA8eHXYYtqslMgWtRTIkSHfey7?purpose=fullsize)

![Image](https://images.openai.com/static-rsc-4/QsIqfDGQ8A-W5kSkm-I2YtlelcxHits863DZx4BuZ1QrO2wjfyw5cuA3Cp5QSQFn1htzIg746mEnTm4iz8FVQjlJdG_nMq-DVU7XZcpm7FnkrR6XFV6bCSG1QmbEmr_ppa37C1MolhwjYo60Vb9runDtOiw3nFmFUudYuxMkaslmAiIyxdFW4Wvu53wkyaBM?purpose=fullsize)

## 1. Final project architecture

I recommend this architecture:

```text
                    STM32MP157-DK2
                          │
                    STM32MP157 SoC
                          │
              ┌───────────┴───────────┐
              │                       │
          Cortex-A7               Cortex-M4
              │
          DDR / Linux
              │
        ┌─────┴─────┐
        │           │
      U-Boot     Device Tree
        │           │
        └─────┬─────┘
              │
        Linux Kernel
              │
       STM32 GPIO Driver
              │
        GPIO Subsystem
              │
       /dev/gpiochipN
              │
        ┌─────┴─────┐
        │           │
     libgpiod    Legacy Sysfs
        │           │
        └─────┬─────┘
              │
       GPIO Application
              │
       ┌──────┼──────┐
       │      │      │
      LED   Button  IRQ
       │      │      │
       └──────┴──────┘
              │
       Physical GPIO
```

And separately:

```text
             Development PC
                   │
          ┌────────┴────────┐
          │                 │
       Yocto/OpenSTLinux   Simulator
          │                 │
          ▼                 ▼
       SD/eMMC          Virtual GPIO
          │
          ▼
    STM32MP157-DK2
```

---

# 2. Repository structure

I would change your current repository to this:

```text
Virtual_Sysfs_GPIO/
│
├── README.md
├── LICENSE
├── Makefile
├── CMakeLists.txt
│
├── docs/
│   ├── architecture.md
│   ├── gpio-linux-subsystem.md
│   ├── gpio-sysfs.md
│   ├── gpio-libgpiod.md
│   ├── gpio-driver.md
│   ├── device-tree.md
│   ├── stm32mp157-gpio.md
│   ├── boot-flow.md
│   └── troubleshooting.md
│
├── application/
│   ├── include/
│   │   ├── gpio_common.h
│   │   ├── gpio_sysfs.h
│   │   └── gpio_libgpiod.h
│   │
│   ├── src/
│   │   ├── gpio-sysfs.c
│   │   ├── gpio-libgpiod.c
│   │   └── gpio-common.c
│   │
│   └── examples/
│       ├── led_blink.c
│       ├── button.c
│       ├── button_irq.c
│       └── gpio_toggle.c
│
├── kernel/
│   ├── driver/
│   │   ├── virtual_gpio.c
│   │   ├── virtual_gpio.h
│   │   └── Makefile
│   │
│   └── patches/
│       └── README.md
│
├── device-tree/
│   ├── stm32mp157-gpio-test.dts
│   ├── stm32mp157-gpio-test-overlay.dts
│   └── README.md
│
├── simulator/
│   ├── simulator.py
│   ├── gpio_model.py
│   ├── gpio_events.py
│   └── config.json
│
├── tests/
│   ├── unit/
│   │   └── test_gpio.c
│   ├── integration/
│   │   └── test_gpio_hw.sh
│   ├── simulation/
│   │   └── test_simulator.sh
│   └── test.sh
│
├── scripts/
│   ├── build.sh
│   ├── clean.sh
│   ├── deploy.sh
│   ├── flash_sd.sh
│   ├── gpio_info.sh
│   └── hardware_test.sh
│
├── yocto/
│   └── meta-virtual-gpio/
│       ├── conf/
│       │   └── layer.conf
│       │
│       ├── recipes-apps/
│       │   └── virtual-gpio/
│       │       ├── virtual-gpio.bb
│       │       └── files/
│       │
│       ├── recipes-kernel/
│       │   └── virtual-gpio/
│       │       └── virtual-gpio.bb
│       │
│       └── README.md
│
├── configs/
│   ├── stm32mp157_gpio_defconfig
│   └── gpio-test-config.json
│
└── examples/
    ├── led/
    ├── button/
    ├── interrupt/
    └── pwm/
```

This gives you a **proper BSP/application/kernel project**, not just a GPIO C program.

---

# 3. Divide the project into 7 layers

The cleanest way to develop this is to divide it into seven stages.

### Layer 1 — Hardware

STM32MP157-DK2:

```text
STM32MP157
   │
   ├── GPIOA
   ├── GPIOB
   ├── GPIOC
   ├── GPIOD
   ├── GPIOE
   ├── GPIOF
   ├── GPIOG
   ├── GPIOH
   └── GPIOI
```

You select a few safe GPIOs for testing.

For example:

```text
GPIO output → LED
GPIO input  → Button
GPIO IRQ   → Button interrupt
```

**Important:** don't blindly use Raspberry Pi numbers such as GPIO17/GPIO18 on STM32MP157. STM32 GPIOs are normally identified by **port + pin**, e.g. `GPIOA 5`, and the Linux userspace interface exposes them as GPIO line offsets through a GPIO chip.

---

# 4. Layer 2 — Bootloader

Your project should document:

```text
Power ON
   ↓
STM32 BootROM
   ↓
TF-A
   ↓
U-Boot
   ↓
Linux Kernel
```

U-Boot's job includes:

```text
Initialize DDR
Initialize storage
Load kernel
Load Device Tree
Set boot arguments
Start Linux
```

You don't need to modify U-Boot initially.

First prove:

```text
STM32MP157-DK2
       ↓
U-Boot
       ↓
Linux
       ↓
login prompt
```

Then continue to GPIO.

---

# 5. Layer 3 — Linux Kernel

Now verify that Linux recognizes the STM32 GPIO controllers.

On the board:

```bash
ls /dev/gpiochip*
```

Then:

```bash
gpiodetect
```

You should get GPIO controllers similar to:

```text
gpiochip0
gpiochip1
gpiochip2
...
```

Then:

```bash
gpioinfo
```

This is important because you should **not assume `gpiochip0` corresponds to a particular STM32 GPIO port**.

You identify it using the chip information and line names.

---

# 6. Layer 4 — Device Tree

This is where your project becomes much more interesting from a BSP perspective.

Create something like:

```text
device-tree/
│
├── stm32mp157-gpio-test.dts
└── README.md
```

Conceptually:

```dts
gpio_test {
        compatible = "gpio-test";

        led-gpios = <...>;
        button-gpios = <...>;
};
```

The actual GPIO controller and pin configuration should follow the STM32MP157 board's existing DTS/DTSI definitions.

The Device Tree describes:

```text
Hardware
   │
   ├── GPIO controller
   ├── GPIO pin
   ├── pinmux
   ├── pull-up/down
   ├── drive configuration
   └── interrupt
```

Then:

```text
Device Tree
     ↓
Kernel parses DT
     ↓
GPIO controller registered
     ↓
GPIO consumer driver/application
```

---

# 7. Layer 5 — libgpiod

This should be your **primary modern userspace interface**.

Your application becomes:

```text
gpio-libgpiod.c
       │
       ▼
    libgpiod
       │
       ▼
 /dev/gpiochipN
       │
       ▼
Linux GPIO subsystem
       │
       ▼
STM32 GPIO controller
       │
       ▼
Physical GPIO
```

For example:

```bash
gpiodetect
gpioinfo
gpioget
gpioset
gpiomon
```

Your C program should eventually support:

```bash
./gpio-control \
    --chip gpiochip0 \
    --line <offset> \
    --direction output \
    --value 1
```

And:

```bash
./gpio-control \
    --chip gpiochip0 \
    --line <offset> \
    --direction input
```

And interrupt monitoring:

```bash
./gpio-control \
    --chip gpiochip0 \
    --line <offset> \
    --edge both
```

---

# 8. Layer 6 — Sysfs

Keep your existing:

```text
gpio-sysfs.c
```

but change its purpose.

Instead of presenting Sysfs as the primary interface, document it as:

```text
              GPIO Userspace
                    │
         ┌──────────┴──────────┐
         │                     │
      libgpiod             Sysfs GPIO
      MODERN                LEGACY
         │                     │
         ▼                     ▼
 /dev/gpiochipN       /sys/class/gpio
```

This gives your project an excellent **legacy vs modern Linux GPIO comparison**.

Also, because sysfs GPIO has been deprecated in favor of the GPIO character-device interface, don't design new functionality around sysfs.

---

# 9. Layer 7 — Your simulator

Your existing:

```text
simulator.py
```

should remain.

But make it a **hardware-independent testing backend**.

Architecture:

```text
                    GPIO Application
                           │
                ┌──────────┴──────────┐
                │                     │
          Hardware backend      Simulation backend
                │                     │
                ▼                     ▼
        STM32MP157 GPIO        simulator.py
                │                     │
                ▼                     ▼
             LED/Button          Virtual GPIO
```

This means you can develop on Ubuntu without the board.

For example:

```bash
./gpio-control --simulate \
               --pin 17 \
               --direction out \
               --value 1
```

Then later:

```bash
./gpio-control --hardware \
               --chip gpiochipX \
               --line Y \
               --value 1
```

Same application concept, different backend.

---

# 10. GPIO test plan

I would make testing progressive.

### Test 1 — GPIO discovery

```bash
gpiodetect
```

Expected:

```text
GPIO chips detected
```

### Test 2 — GPIO information

```bash
gpioinfo
```

Verify:

```text
line
consumer
direction
active-low
```

### Test 3 — Output

```text
Application
    ↓
libgpiod
    ↓
GPIO
    ↓
LED ON
```

### Test 4 — Input

```text
Button
   ↓
GPIO
   ↓
libgpiod
   ↓
Application
```

### Test 5 — Interrupt

```text
Button press
     ↓
GPIO edge
     ↓
STM32 interrupt
     ↓
Linux IRQ
     ↓
GPIO event
     ↓
gpiomon/libgpiod
     ↓
Application
```

### Test 6 — Debouncing

```text
Button
   ↓
Multiple transitions
   ↓
Debounce logic
   ↓
Single logical event
```

### Test 7 — Simulation

Run the exact same logical tests without hardware.

---

# 11. Add a kernel driver

This is optional initially, but **I strongly recommend adding it** if your goal is to make this a serious Linux kernel/BSP portfolio project.

Create:

```text
kernel/
└── driver/
    ├── virtual_gpio.c
    ├── virtual_gpio.h
    └── Makefile
```

The driver can demonstrate:

```text
module_init()
module_exit()

platform_driver
probe()
remove()

devm_gpiod_get()
gpiod_direction_output()
gpiod_set_value()
gpiod_get_value()
gpiod_to_irq()
request_threaded_irq()
```

Then:

```text
Device Tree
     ↓
platform device
     ↓
driver probe()
     ↓
GPIO descriptors
     ↓
GPIO configuration
     ↓
IRQ
```

That is much more valuable for an embedded Linux interview than only showing `gpioset`.

---

# 12. Yocto integration

This is where I would take your project one step further.

Create:

```text
yocto/
└── meta-virtual-gpio/
```

Your custom layer contains:

```text
meta-virtual-gpio/
│
├── conf/
│   └── layer.conf
│
├── recipes-apps/
│   └── virtual-gpio/
│       ├── virtual-gpio.bb
│       └── files/
│           ├── gpio-control.c
│           ├── Makefile
│           └── gpio-test.sh
│
└── recipes-kernel/
    └── virtual-gpio/
        └── virtual-gpio.bb
```

Then the Yocto build becomes:

```text
OpenSTLinux / Yocto
        │
        ├── Linux kernel
        ├── U-Boot
        ├── Device Tree
        ├── RootFS
        │
        └── meta-virtual-gpio
                  │
                  ├── gpio application
                  ├── GPIO tests
                  └── kernel driver
```

Then the generated image contains your application automatically.

---

# 13. Build flow

Your final development flow should be:

```text
                    DEVELOPMENT PC
                           │
                           ▼
                    Git Repository
                           │
             ┌─────────────┼─────────────┐
             ▼             ▼             ▼
           CMake         Yocto        Simulator
             │             │             │
             ▼             ▼             ▼
       GPIO Application  Linux Image  Virtual GPIO
                           │
                           ▼
                    SD / eMMC / Boot
                           │
                           ▼
                   STM32MP157-DK2
                           │
                           ▼
                        U-Boot
                           │
                           ▼
                    Linux Kernel
                           │
                           ▼
                    Device Tree
                           │
                           ▼
                  STM32 GPIO Driver
                           │
                           ▼
                   GPIO Framework
                           │
                           ▼
                    /dev/gpiochipN
                           │
                           ▼
                       libgpiod
                           │
                           ▼
                    Your Application
                           │
                  ┌────────┼────────┐
                  ▼        ▼        ▼
                 LED     Button     IRQ
```

---

# 14. GitHub project milestones

Don't try to implement everything at once.

I would use these milestones:

### Phase 1 — Existing application

```text
C GPIO application
Makefile
CMake
Simulator
Unit tests
```

### Phase 2 — STM32MP157 hardware

```text
STM32MP157-DK2
OpenSTLinux
U-Boot
Linux
GPIO discovery
```

### Phase 3 — GPIO hardware

```text
libgpiod
LED
Button
GPIO input/output
```

### Phase 4 — Interrupts

```text
GPIO IRQ
Rising edge
Falling edge
Both edges
Debouncing
```

### Phase 5 — Device Tree

```text
GPIO pinmux
GPIO descriptors
GPIO consumer node
Interrupt configuration
```

### Phase 6 — Kernel driver

```text
Platform driver
probe/remove
GPIO descriptor API
IRQ handling
```

### Phase 7 — Yocto

```text
meta-virtual-gpio
Application recipe
Driver recipe
Image integration
```

### Phase 8 — Automated testing

```text
Unit tests
Simulation tests
Hardware tests
GPIO loopback tests
CI/CD
```

---

# 15. Final project architecture for GitHub

Your repository will eventually demonstrate:

```text
                  Virtual GPIO Simulator
                           │
                           │
                           ▼
                    GPIO Application
                           │
              ┌────────────┴────────────┐
              │                         │
          Simulation                 Hardware
              │                         │
              ▼                         ▼
       Python Simulator          STM32MP157-DK2
                                        │
                                        ▼
                                 Device Tree
                                        │
                                        ▼
                                  Linux Kernel
                                        │
                                        ▼
                                GPIO Framework
                                        │
                            ┌───────────┴───────────┐
                            │                       │
                        libgpiod                 Sysfs
                         modern                  legacy
                            │                       │
                            └───────────┬───────────┘
                                        ▼
                                  GPIO Hardware
                                        │
                            ┌───────────┼───────────┐
                            ▼           ▼           ▼
                           LED        Button       IRQ
```

## The most important change

I **would not call the final project simply "Virtual Sysfs GPIO" anymore**.

The STM32MP157 implementation is better represented as something like:

**`STM32MP157_Linux_GPIO_Framework`**

or

**`STM32MP157_GPIO_Control_and_Simulation`**

because the final project covers much more than Sysfs:

**GPIO simulation + libgpiod + legacy Sysfs + Device Tree + Linux GPIO framework + kernel driver + interrupts + Yocto + real STM32MP157 hardware.**

That would make it a much stronger **Embedded Linux BSP / Linux Kernel / Device Driver portfolio project**.







