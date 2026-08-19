Make it executable
chmod +x tests/integration/test_gpio_hw.sh
Run on STM32MP157-DK2
./tests/integration/test_gpio_hw.sh

Or specify your actual GPIO:

GPIO_CHIP=gpiochip0 GPIO_LINE=13 \
./tests/integration/test_gpio_hw.sh
Important for your project

The exact GPIO_LINE=13 is only an example. On the STM32MP157-DK2, the GPIO line must correspond to the GPIO you actually expose/configure through the Device Tree.

The overall project flow should now be:

STM32MP157-DK2
│
├── application/
│   ├── include/
│   └── src/
│
├── configs/
│
├── device-tree/
│
├── docs/
│
├── examples/
│   ├── button/
│   ├── interrupt/
│   ├── led/
│   └── pwm/
│
├── kernel/
│   ├── driver/
│   └── patches/
│
├── scripts/
│
├── simulator/
│
└── tests/
    └── integration/
        └── test_gpio_hw.sh

This makes test_gpio_hw.sh the final real-board validation layer: application → libgpiod/sysfs → Linux GPIO subsystem → Device Tree → STM32MP157 GPIO controller → physical GPIO pin.

