# STM32MP157-DK2 GPIO Device Tree

## Overview

This directory contains the Device Tree configuration used by the
STM32MP157-DK2 GPIO Control and Simulator project.

The Device Tree describes:

- GPIO controllers
- GPIO pins
- GPIO polarity
- GPIO direction
- GPIO LEDs
- GPIO buttons
- GPIO interrupts
- GPIO peripheral ownership

The Linux GPIO subsystem reads this information during kernel boot.

---

## Files

```text
device-tree/
├── README.md
├── stm32mp157-gpio-test.dts

