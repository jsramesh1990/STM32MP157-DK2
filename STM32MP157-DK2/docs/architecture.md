# STM32MP157-DK2 GPIO Project Architecture

## 1. Overview

This document describes the complete architecture of the
STM32MP157-DK2 GPIO Control and Testing project.

The project demonstrates GPIO control from the Linux userspace down
to the STM32MP157 hardware GPIO controller.

The project supports:

- Linux GPIO framework
- STM32 GPIO controller
- Device Tree configuration
- GPIO character device
- libgpiod
- Legacy GPIO sysfs
- GPIO input/output
- GPIO interrupt/event detection
- LED control
- Button control
- GPIO simulation
- Automated testing

---

# 2. Hardware Platform

## Board

STM32MP157-DK2 Discovery Kit

## Processor

STM32MP157

## Application Processor

Dual Cortex-A7

## Real-Time Processor

Cortex-M4

## Operating System

Embedded Linux

## Main GPIO Software Stack

```text
Application
    |
    v
GPIO Application API
    |
    +----------------------+
    |                      |
    v                      v
libgpiod                GPIO Sysfs
    |                      |
    v                      v
GPIO Character Device   /sys/class/gpio
    |                      |
    +----------+-----------+
               |
               v
          Linux GPIOLIB
               |
               v
       STM32 GPIO Driver
               |
               v
       STM32 GPIO Controller
               |
               v
          Phy
