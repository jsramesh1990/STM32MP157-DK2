Make executable
chmod +x STM32MP157-DK2/tests/simulation/test_simulator.sh
Run

From the project root:

./tests/simulation/test_simulator.sh

The purpose of the two test directories is now clear:

tests/
│
├── integration/
│   └── test_gpio_hw.sh
│       └── Tests REAL STM32MP157-DK2 GPIO hardware
│
└── simulation/
    └── test_simulator.sh
        └── Tests GPIO without hardware

So your validation flow becomes:

                 STM32MP157-DK2 GPIO PROJECT
                           │
              ┌────────────┴────────────┐
              │                         │
        SIMULATION TEST             HARDWARE TEST
              │                         │
              ▼                         ▼
 test_simulator.sh             test_gpio_hw.sh
              │                         │
              ▼                         ▼
   simulator/*.py              Linux GPIO subsystem
              │                         │
              ▼                         ▼
     Virtual GPIO             STM32MP157 GPIO
              │                         │
              ▼                         ▼
       PASS / FAIL             PASS / FAIL

This separation is good for your project because you can develop and test the GPIO application on a PC first, then deploy the same functionality to the STM32MP157-DK2 and run the hardware integration test.
