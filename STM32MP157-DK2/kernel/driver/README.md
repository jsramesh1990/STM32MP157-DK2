The important runtime flow is:

STM32MP157-DK2
      │
      ▼
BootROM
      │
      ▼
TF-A / U-Boot
      │
      ▼
Linux Kernel
      │
      ▼
Device Tree
      │
      ▼
GPIO Subsystem
      │
      ├───────────────┐
      ▼               ▼
Physical GPIO     virtual_gpio
                    driver
                       │
                       ▼
                 gpiochipX
                       │
                       ▼
                  libgpiod
                       │
                       ▼
             Your C applications
                       │
          ┌────────────┼────────────┐
          ▼            ▼            ▼
       LED Blink    Button IRQ     PWM

One important point: this virtual_gpio.c is a virtual GPIO controller, not the STM32MP157's physical GPIO driver. For your project, that is useful because you can demonstrate the complete Linux GPIO subsystem → kernel driver → libgpiod → application flow. For actual LEDs/buttons on the DK2, the STM32 GPIO controller and Device Tree must also be configured appropriately.
