Yes. For the **STM32MP157-DK2** project, the `simulator/` directory can be completely self-contained and should simulate GPIO behavior without requiring the physical board.

Recommended structure:

```text
STM32MP157-DK2/
└── simulator/
    ├── config.json
    ├── gpio_events.py
    ├── gpio_model.py
    └── simulator.py
```

Below are the **full contents of all 4 files**.

---

# 1. `simulator/config.json`

```json
{
    "simulator": {
        "name": "STM32MP157-DK2 GPIO Simulator",
        "version": "1.0",
        "log_level": "INFO",
        "poll_interval_ms": 100
    },

    "board": {
        "name": "STM32MP157-DK2",
        "soc": "STM32MP157",
        "gpio_chips": 1
    },

    "gpio": {
        "chips": [
            {
                "name": "gpiochip0",
                "label": "STM32MP157 GPIO",
                "num_lines": 16,

                "lines": [
                    {
                        "offset": 0,
                        "name": "GPIO0",
                        "direction": "input",
                        "active_low": false,
                        "value": 0,
                        "bias": "pull_up"
                    },
                    {
                        "offset": 1,
                        "name": "GPIO1",
                        "direction": "output",
                        "active_low": false,
                        "value": 0,
                        "bias": "disabled"
                    },
                    {
                        "offset": 2,
                        "name": "GPIO2",
                        "direction": "input",
                        "active_low": false,
                        "value": 0,
                        "bias": "pull_up"
                    },
                    {
                        "offset": 3,
                        "name": "GPIO3",
                        "direction": "output",
                        "active_low": false,
                        "value": 0,
                        "bias": "disabled"
                    },
                    {
                        "offset": 4,
                        "name": "GPIO4",
                        "direction": "input",
                        "active_low": false,
                        "value": 0,
                        "bias": "pull_down"
                    },
                    {
                        "offset": 5,
                        "name": "GPIO5",
                        "direction": "output",
                        "active_low": false,
                        "value": 0,
                        "bias": "disabled"
                    },
                    {
                        "offset": 6,
                        "name": "GPIO6",
                        "direction": "input",
                        "active_low": false,
                        "value": 0,
                        "bias": "pull_up"
                    },
                    {
                        "offset": 7,
                        "name": "GPIO7",
                        "direction": "output",
                        "active_low": false,
                        "value": 0,
                        "bias": "disabled"
                    },
                    {
                        "offset": 8,
                        "name": "GPIO8",
                        "direction": "input",
                        "active_low": false,
                        "value": 0,
                        "bias": "disabled"
                    },
                    {
                        "offset": 9,
                        "name": "GPIO9",
                        "direction": "output",
                        "active_low": false,
                        "value": 0,
                        "bias": "disabled"
                    },
                    {
                        "offset": 10,
                        "name": "GPIO10",
                        "direction": "input",
                        "active_low": false,
                        "value": 0,
                        "bias": "disabled"
                    },
                    {
                        "offset": 11,
                        "name": "GPIO11",
                        "direction": "output",
                        "active_low": false,
                        "value": 0,
                        "bias": "disabled"
                    },
                    {
                        "offset": 12,
                        "name": "GPIO12",
                        "direction": "input",
                        "active_low": false,
                        "value": 0,
                        "bias": "disabled"
                    },
                    {
                        "offset": 13,
                        "name": "GPIO13",
                        "direction": "output",
                        "active_low": false,
                        "value": 0,
                        "bias": "disabled"
                    },
                    {
                        "offset": 14,
                        "name": "GPIO14",
                        "direction": "input",
                        "active_low": false,
                        "value": 0,
                        "bias": "disabled"
                    },
                    {
                        "offset": 15,
                        "name": "GPIO15",
                        "direction": "output",
                        "active_low": false,
                        "value": 0,
                        "bias": "disabled"
                    }
                ]
            }
        ]
    },

    "leds": [
        {
            "name": "LED1",
            "gpio": 1,
            "active_high": true
        },
        {
            "name": "LED2",
            "gpio": 3,
            "active_high": true
        },
        {
            "name": "LED3",
            "gpio": 5,
            "active_high": true
        },
        {
            "name": "LED4",
            "gpio": 7,
            "active_high": true
        }
    ],

    "buttons": [
        {
            "name": "BUTTON1",
            "gpio": 0,
            "active_low": true
        },
        {
            "name": "BUTTON2",
            "gpio": 2,
            "active_low": true
        },
        {
            "name": "BUTTON3",
            "gpio": 4,
            "active_low": true
        },
        {
            "name": "BUTTON4",
            "gpio": 6,
            "active_low": true
        }
    ],

    "interrupts": {
        "enabled": true,
        "default_edge": "both"
    },

    "pwm": {
        "enabled": true,
        "frequency_hz": 1000,
        "default_duty_cycle": 50
    }
}
```

---

# 2. `simulator/gpio_events.py`

This file handles GPIO events such as:

* Rising edge
* Falling edge
* Both edges
* GPIO value change
* Button press
* Button release

```python
#!/usr/bin/env python3

"""
gpio_events.py

GPIO event and interrupt handling for the
STM32MP157-DK2 GPIO simulator.
"""

import time
import threading
from enum import Enum
from dataclasses import dataclass
from typing import Callable, Optional


class GPIOEventType(Enum):
    """
    GPIO event types.
    """

    VALUE_CHANGE = "value_change"
    RISING_EDGE = "rising"
    FALLING_EDGE = "falling"
    BOTH_EDGES = "both"


@dataclass
class GPIOEvent:
    """
    Represents a GPIO event.
    """

    gpio: int
    old_value: int
    new_value: int
    event_type: GPIOEventType
    timestamp: float

    @property
    def edge(self):
        if self.new_value > self.old_value:
            return "rising"

        if self.new_value < self.old_value:
            return "falling"

        return "none"

    def __str__(self):
        timestamp = time.strftime(
            "%H:%M:%S",
            time.localtime(self.timestamp)
        )

        return (
            f"[{timestamp}] "
            f"GPIO{self.gpio}: "
            f"{self.old_value} -> {self.new_value} "
            f"({self.edge})"
        )


class GPIOEventManager:
    """
    GPIO event manager.

    Allows applications to register callbacks for GPIO
    rising/falling/both edge events.
    """

    def __init__(self):
        self._callbacks = {}
        self._lock = threading.Lock()

    def register(
        self,
        gpio: int,
        callback: Callable[[GPIOEvent], None],
        edge: str = "both"
    ):
        """
        Register callback for a GPIO.

        edge:
            rising
            falling
            both
            none
        """

        edge = edge.lower()

        if edge not in ("rising", "falling", "both", "none"):
            raise ValueError(
                "Invalid edge type: "
                "use rising, falling, both or none"
            )

        with self._lock:

            if gpio not in self._callbacks:
                self._callbacks[gpio] = []

            self._callbacks[gpio].append(
                {
                    "callback": callback,
                    "edge": edge
                }
            )

    def unregister(
        self,
        gpio: int,
        callback: Optional[Callable] = None
    ):
        """
        Remove callback.

        If callback is None, all callbacks for the GPIO
        are removed.
        """

        with self._lock:

            if gpio not in self._callbacks:
                return

            if callback is None:
                del self._callbacks[gpio]
                return

            self._callbacks[gpio] = [
                item
                for item in self._callbacks[gpio]
                if item["callback"] != callback
            ]

            if not self._callbacks[gpio]:
                del self._callbacks[gpio]

    def _edge_matches(
        self,
        configured_edge: str,
        event_edge: str
    ):
        if configured_edge == "both":
            return event_edge in ("rising", "falling")

        return configured_edge == event_edge

    def dispatch(self, event: GPIOEvent):
        """
        Dispatch event to registered callbacks.
        """

        with self._lock:
            callbacks = list(
                self._callbacks.get(event.gpio, [])
            )

        for item in callbacks:

            configured_edge = item["edge"]

            if configured_edge == "none":
                continue

            if self._edge_matches(
                configured_edge,
                event.edge
            ):
                try:
                    item["callback"](event)

                except Exception as exc:
                    print(
                        f"[EVENT ERROR] "
                        f"GPIO{event.gpio}: {exc}"
                    )


class Debouncer:
    """
    Simple software GPIO debounce mechanism.
    """

    def __init__(self, debounce_ms=50):

        self.debounce_ms = debounce_ms
        self._last_event = {}
        self._lock = threading.Lock()

    def accept(self, gpio: int):

        now = time.monotonic()

        with self._lock:

            last = self._last_event.get(gpio)

            if last is not None:

                elapsed_ms = (
                    now - last
                ) * 1000.0

                if elapsed_ms < self.debounce_ms:
                    return False

            self._last_event[gpio] = now

            return True


def create_gpio_event(
    gpio: int,
    old_value: int,
    new_value: int
):
    """
    Create GPIOEvent based on value transition.
    """

    if old_value == new_value:
        event_type = GPIOEventType.VALUE_CHANGE

    elif new_value > old_value:
        event_type = GPIOEventType.RISING_EDGE

    else:
        event_type = GPIOEventType.FALLING_EDGE

    return GPIOEvent(
        gpio=gpio,
        old_value=old_value,
        new_value=new_value,
        event_type=event_type,
        timestamp=time.time()
    )
```

---

# 3. `simulator/gpio_model.py`

This is the **core GPIO hardware model**.

It models:

```text
GPIO Chip
   |
   +--- GPIO lines
          |
          +--- direction
          +--- value
          +--- bias
          +--- active_low
          +--- interrupt
```

```python
#!/usr/bin/env python3

"""
gpio_model.py

Virtual GPIO hardware model for STM32MP157-DK2.

This module simulates:

    - GPIO input
    - GPIO output
    - GPIO direction
    - GPIO value
    - GPIO active-low
    - GPIO pull-up
    - GPIO pull-down
    - GPIO edge detection
    - GPIO interrupts
    - LED state
    - Button state
"""

import json
import threading

from gpio_events import (
    GPIOEventManager,
    GPIOEvent,
    create_gpio_event,
    Debouncer
)


class GPIOLine:
    """
    Represents one GPIO line.
    """

    VALID_DIRECTIONS = (
        "input",
        "output"
    )

    VALID_BIAS = (
        "disabled",
        "pull_up",
        "pull_down"
    )

    def __init__(
        self,
        offset,
        name="",
        direction="input",
        value=0,
        active_low=False,
        bias="disabled"
    ):

        self.offset = int(offset)
        self.name = name or f"GPIO{offset}"

        self.direction = direction
        self.value = int(value)

        self.active_low = bool(active_low)
        self.bias = bias

        self.requested = False

        self._lock = threading.Lock()

        self._validate()

    def _validate(self):

        if self.direction not in self.VALID_DIRECTIONS:
            raise ValueError(
                f"Invalid GPIO direction: {self.direction}"
            )

        if self.bias not in self.VALID_BIAS:
            raise ValueError(
                f"Invalid GPIO bias: {self.bias}"
            )

        if self.value not in (0, 1):
            raise ValueError(
                "GPIO value must be 0 or 1"
            )

    def request(self):

        with self._lock:

            if self.requested:
                raise RuntimeError(
                    f"{self.name} is already requested"
                )

            self.requested = True

    def release(self):

        with self._lock:
            self.requested = False

    def set_direction(self, direction):

        if direction not in self.VALID_DIRECTIONS:
            raise ValueError(
                "Direction must be input or output"
            )

        with self._lock:
            self.direction = direction

    def get_direction(self):

        with self._lock:
            return self.direction

    def set_value(self, value):

        value = int(value)

        if value not in (0, 1):
            raise ValueError(
                "GPIO value must be 0 or 1"
            )

        with self._lock:

            if self.direction != "output":
                raise RuntimeError(
                    f"{self.name} is not configured as output"
                )

            old_value = self.value
            self.value = value

            return old_value, value

    def simulate_input(self, value):

        value = int(value)

        if value not in (0, 1):
            raise ValueError(
                "GPIO input value must be 0 or 1"
            )

        with self._lock:

            if self.direction != "input":
                raise RuntimeError(
                    f"{self.name} is not configured as input"
                )

            old_value = self.value
            self.value = value

            return old_value, value

    def get_value(self):

        with self._lock:

            value = self.value

            if self.active_low:
                value = 0 if value else 1

            return value

    def get_raw_value(self):

        with self._lock:
            return self.value

    def configure(
        self,
        direction=None,
        bias=None,
        active_low=None
    ):

        with self._lock:

            if direction is not None:
                if direction not in self.VALID_DIRECTIONS:
                    raise ValueError(
                        "Invalid direction"
                    )

                self.direction = direction

            if bias is not None:
                if bias not in self.VALID_BIAS:
                    raise ValueError(
                        "Invalid bias"
                    )

                self.bias = bias

            if active_low is not None:
                self.active_low = bool(active_low)

    def __repr__(self):

        return (
            f"GPIOLine("
            f"offset={self.offset}, "
            f"name='{self.name}', "
            f"direction='{self.direction}', "
            f"value={self.value}, "
            f"active_low={self.active_low}, "
            f"bias='{self.bias}'"
            f")"
        )


class GPIOChip:
    """
    Represents /dev/gpiochipX.
    """

    def __init__(
        self,
        name,
        label,
        num_lines
    ):

        self.name = name
        self.label = label
        self.num_lines = num_lines

        self.lines = {}

        self.event_manager = GPIOEventManager()
        self.debouncer = Debouncer(50)

        self._lock = threading.Lock()

    def add_line(self, line: GPIOLine):

        if line.offset >= self.num_lines:
            raise ValueError(
                f"GPIO offset {line.offset} "
                f"is outside chip range"
            )

        self.lines[line.offset] = line

    def get_line(self, offset):

        offset = int(offset)

        if offset not in self.lines:
            raise KeyError(
                f"GPIO line {offset} not found"
            )

        return self.lines[offset]

    def request_line(self, offset):

        line = self.get_line(offset)

        line.request()

        return line

    def release_line(self, offset):

        line = self.get_line(offset)

        line.release()

    def set_value(self, offset, value):

        line = self.get_line(offset)

        old_value, new_value = line.set_value(value)

        if old_value != new_value:
            self._generate_event(
                offset,
                old_value,
                new_value
            )

    def simulate_input(self, offset, value):

        line = self.get_line(offset)

        old_value, new_value = line.simulate_input(value)

        if old_value != new_value:

            self._generate_event(
                offset,
                old_value,
                new_value
            )

    def get_value(self, offset):

        return self.get_line(offset).get_value()

    def get_raw_value(self, offset):

        return self.get_line(offset).get_raw_value()

    def set_direction(self, offset, direction):

        self.get_line(offset).set_direction(direction)

    def _generate_event(
        self,
        offset,
        old_value,
        new_value
    ):

        if not self.debouncer.accept(offset):
            return

        event = create_gpio_event(
            offset,
            old_value,
            new_value
        )

        self.event_manager.dispatch(event)

    def register_interrupt(
        self,
        offset,
        callback,
        edge="both"
    ):

        self.event_manager.register(
            offset,
            callback,
            edge
        )

    def unregister_interrupt(
        self,
        offset,
        callback=None
    ):

        self.event_manager.unregister(
            offset,
            callback
        )

    def gpio_info(self):

        result = []

        for offset in sorted(self.lines):

            line = self.lines[offset]

            result.append(
                {
                    "offset": line.offset,
                    "name": line.name,
                    "direction": line.direction,
                    "value": line.get_value(),
                    "raw_value": line.get_raw_value(),
                    "bias": line.bias,
                    "active_low": line.active_low,
                    "requested": line.requested
                }
            )

        return result


class GPIOSimulatorModel:
    """
    Top-level GPIO simulator model.

    Represents the complete GPIO hardware of the
    STM32MP157-DK2 simulation environment.
    """

    def __init__(self):

        self.chips = {}

        self.leds = {}
        self.buttons = {}

        self._lock = threading.Lock()

    def add_chip(self, chip: GPIOChip):

        self.chips[chip.name] = chip

    def get_chip(self, name="gpiochip0"):

        if name not in self.chips:
            raise KeyError(
                f"GPIO chip '{name}' not found"
            )

        return self.chips[name]

    def add_led(
        self,
        name,
        gpio,
        active_high=True
    ):

        self.leds[name] = {
            "gpio": gpio,
            "active_high": active_high,
            "state": 0
        }

    def add_button(
        self,
        name,
        gpio,
        active_low=True
    ):

        self.buttons[name] = {
            "gpio": gpio,
            "active_low": active_low,
            "pressed": False
        }

    def set_gpio(self, gpio, value):

        chip = self.get_chip()

        chip.set_value(gpio, value)

        self._update_leds(gpio)

    def get_gpio(self, gpio):

        chip = self.get_chip()

        return chip.get_value(gpio)

    def press_button(self, name):

        if name not in self.buttons:
            raise KeyError(
                f"Button '{name}' not found"
            )

        button = self.buttons[name]

        gpio = button["gpio"]

        button["pressed"] = True

        # Active-low button:
        # pressed = 0
        # released = 1
        value = 0 if button["active_low"] else 1

        self.get_chip().simulate_input(
            gpio,
            value
        )

    def release_button(self, name):

        if name not in self.buttons:
            raise KeyError(
                f"Button '{name}' not found"
            )

        button = self.buttons[name]

        gpio = button["gpio"]

        button["pressed"] = False

        value = 1 if button["active_low"] else 0

        self.get_chip().simulate_input(
            gpio,
            value
        )

    def toggle_button(self, name):

        if self.buttons[name]["pressed"]:
            self.release_button(name)

        else:
            self.press_button(name)

    def _update_leds(self, gpio):

        for name, led in self.leds.items():

            if led["gpio"] != gpio:
                continue

            value = self.get_gpio(gpio)

            if led["active_high"]:
                led["state"] = value

            else:
                led["state"] = 0 if value else 1

    def get_led_state(self, name):

        if name not in self.leds:
            raise KeyError(
                f"LED '{name}' not found"
            )

        led = self.leds[name]

        return led["state"]

    def get_status(self):

        chip = self.get_chip()

        return {
            "chip": chip.name,
            "label": chip.label,
            "lines": chip.gpio_info(),
            "leds": self.leds,
            "buttons": self.buttons
        }


def load_model_from_config(filename):

    """
    Create GPIOSimulatorModel from JSON configuration.
    """

    with open(filename, "r", encoding="utf-8") as file:
        config = json.load(file)

    model = GPIOSimulatorModel()

    gpio_config = config.get("gpio", {})

    for chip_config in gpio_config.get(
        "chips",
        []
    ):

        chip = GPIOChip(
            name=chip_config["name"],
            label=chip_config.get(
                "label",
                chip_config["name"]
            ),
            num_lines=chip_config.get(
                "num_lines",
                32
            )
        )

        for line_config in chip_config.get(
            "lines",
            []
        ):

            line = GPIOLine(
                offset=line_config["offset"],
                name=line_config.get(
                    "name",
                    f"GPIO{line_config['offset']}"
                ),
                direction=line_config.get(
                    "direction",
                    "input"
                ),
                value=line_config.get(
                    "value",
                    0
                ),
                active_low=line_config.get(
                    "active_low",
                    False
                ),
                bias=line_config.get(
                    "bias",
                    "disabled"
                )
            )

            chip.add_line(line)

        model.add_chip(chip)

    for led in config.get("leds", []):

        model.add_led(
            name=led["name"],
            gpio=led["gpio"],
            active_high=led.get(
                "active_high",
                True
            )
        )

    for button in config.get("buttons", []):

        model.add_button(
            name=button["name"],
            gpio=button["gpio"],
            active_low=button.get(
                "active_low",
                True
            )
        )

    return model
```

---

# 4. `simulator/simulator.py`

This is the **main executable simulator**.

It provides a terminal interface:

```text
STM32MP157-DK2 GPIO Simulator

Commands:

info
gpio 1 1
gpio 1 0
read 1

press BUTTON1
release BUTTON1

led
buttons

watch 0 rising
watch 0 falling
watch 0 both

help
quit
```

Full file:

```python
#!/usr/bin/env python3

"""
simulator.py

Main command-line GPIO simulator for STM32MP157-DK2.

Usage:

    python3 simulator.py

    python3 simulator.py --config config.json

The simulator provides:

    GPIO output simulation
    GPIO input simulation
    LED simulation
    Button simulation
    Interrupt simulation
    Rising edge detection
    Falling edge detection
    Both edge detection
    GPIO information
"""

import argparse
import os
import sys
import time

from gpio_model import load_model_from_config


class STM32GPIOApplication:

    def __init__(self, config_file):

        self.config_file = config_file

        print()
        print("=" * 60)
        print("      STM32MP157-DK2 GPIO SIMULATOR")
        print("=" * 60)
        print()

        print(
            f"[INIT] Loading configuration: "
            f"{config_file}"
        )

        self.model = load_model_from_config(
            config_file
        )

        print("[INIT] GPIO model loaded")

        self.running = True

    def print_help(self):

        print()
        print("Available commands:")
        print()
        print("  info")
        print("      Display GPIO chip and line information")
        print()
        print("  read <gpio>")
        print("      Read GPIO value")
        print()
        print("  gpio <gpio> <value>")
        print("      Set GPIO output value")
        print()
        print("  direction <gpio> <input|output>")
        print("      Configure GPIO direction")
        print()
        print("  press <button>")
        print("      Simulate button press")
        print()
        print("  release <button>")
        print("      Simulate button release")
        print()
        print("  button <button>")
        print("      Toggle button state")
        print()
        print("  buttons")
        print("      Display button states")
        print()
        print("  leds")
        print("      Display LED states")
        print()
        print("  watch <gpio> <rising|falling|both>")
        print("      Register GPIO interrupt")
        print()
        print("  unwatch <gpio>")
        print("      Remove GPIO interrupt")
        print()
        print("  help")
        print("      Display this help")
        print()
        print("  quit")
        print("      Exit simulator")
        print()

    def print_info(self):

        status = self.model.get_status()

        print()
        print("GPIO CHIP")
        print("-" * 60)

        print(
            f"Name       : {status['chip']}"
        )

        print(
            f"Label      : {status['label']}"
        )

        print(
            f"GPIO Lines : {len(status['lines'])}"
        )

        print()

        print(
            "OFFSET | NAME       | DIR    | VALUE | BIAS       | REQUESTED"
        )

        print("-" * 60)

        for line in status["lines"]:

            direction = line["direction"]

            print(
                f"{line['offset']:6d} | "
                f"{line['name']:<10} | "
                f"{direction:<6} | "
                f"{line['value']:5d} | "
                f"{line['bias']:<10} | "
                f"{str(line['requested']):<9}"
            )

        print()

    def read_gpio(self, gpio):

        try:

            value = self.model.get_gpio(
                gpio
            )

            print(
                f"GPIO{gpio} = {value}"
            )

        except Exception as exc:

            print(
                f"[ERROR] {exc}"
            )

    def set_gpio(self, gpio, value):

        try:

            self.model.set_gpio(
                gpio,
                value
            )

            print(
                f"GPIO{gpio} <- {value}"
            )

            self.show_led_for_gpio(gpio)

        except Exception as exc:

            print(
                f"[ERROR] {exc}"
            )

    def set_direction(
        self,
        gpio,
        direction
    ):

        try:

            chip = self.model.get_chip()

            chip.set_direction(
                gpio,
                direction
            )

            print(
                f"GPIO{gpio} direction = "
                f"{direction}"
            )

        except Exception as exc:

            print(
                f"[ERROR] {exc}"
            )

    def show_led_for_gpio(self, gpio):

        for name, led in self.model.leds.items():

            if led["gpio"] != gpio:
                continue

            state = self.model.get_led_state(
                name
            )

            if state:
                status = "ON"

            else:
                status = "OFF"

            print(
                f"{name}: {status}"
            )

    def show_leds(self):

        print()
        print("LED STATUS")
        print("-" * 40)

        for name in self.model.leds:

            led = self.model.leds[name]

            state = self.model.get_led_state(
                name
            )

            status = "ON" if state else "OFF"

            print(
                f"{name:<10} "
                f"GPIO{led['gpio']:<3} "
                f"{status}"
            )

        print()

    def show_buttons(self):

        print()
        print("BUTTON STATUS")
        print("-" * 40)

        for name, button in self.model.buttons.items():

            state = (
                "PRESSED"
                if button["pressed"]
                else "RELEASED"
            )

            print(
                f"{name:<10} "
                f"GPIO{button['gpio']:<3} "
                f"{state}"
            )

        print()

    def press_button(self, name):

        try:

            self.model.press_button(name)

            print(
                f"{name}: PRESSED"
            )

        except Exception as exc:

            print(
                f"[ERROR] {exc}"
            )

    def release_button(self, name):

        try:

            self.model.release_button(name)

            print(
                f"{name}: RELEASED"
            )

        except Exception as exc:

            print(
                f"[ERROR] {exc}"
            )

    def toggle_button(self, name):

        try:

            self.model.toggle_button(name)

            state = self.model.buttons[name][
                "pressed"
            ]

            print(
                f"{name}: "
                f"{'PRESSED' if state else 'RELEASED'}"
            )

        except Exception as exc:

            print(
                f"[ERROR] {exc}"
            )

    def interrupt_callback(self, event):

        print()
        print(
            f"[INTERRUPT] {event}"
        )

        print(
            f"             GPIO{event.gpio}: "
            f"{event.old_value} -> "
            f"{event.new_value}"
        )

        print(
            f"             Edge: {event.edge}"
        )

        print(
            f"             Timestamp: "
            f"{event.timestamp:.6f}"
        )

        print()

    def watch_gpio(
        self,
        gpio,
        edge
    ):

        try:

            chip = self.model.get_chip()

            chip.register_interrupt(
                gpio,
                self.interrupt_callback,
                edge
            )

            print(
                f"Interrupt registered:"
                f" GPIO{gpio}, edge={edge}"
            )

        except Exception as exc:

            print(
                f"[ERROR] {exc}"
            )

    def unwatch_gpio(self, gpio):

        try:

            chip = self.model.get_chip()

            chip.unregister_interrupt(
                gpio
            )

            print(
                f"Interrupt removed from GPIO{gpio}"
            )

        except Exception as exc:

            print(
                f"[ERROR] {exc}"
            )

    def execute_command(self, command):

        command = command.strip()

        if not command:
            return

        args = command.split()

        operation = args[0].lower()

        try:

            if operation == "help":

                self.print_help()

            elif operation == "info":

                self.print_info()

            elif operation == "read":

                if len(args) != 2:
                    print(
                        "Usage: read <gpio>"
                    )
                    return

                gpio = int(args[1])

                self.read_gpio(gpio)

            elif operation == "gpio":

                if len(args) != 3:
                    print(
                        "Usage: gpio <gpio> <value>"
                    )
                    return

                gpio = int(args[1])
                value = int(args[2])

                self.set_gpio(
                    gpio,
                    value
                )

            elif operation == "direction":

                if len(args) != 3:
                    print(
                        "Usage: direction "
                        "<gpio> <input|output>"
                    )
                    return

                gpio = int(args[1])
                direction = args[2].lower()

                self.set_direction(
                    gpio,
                    direction
                )

            elif operation == "press":

                if len(args) != 2:
                    print(
                        "Usage: press <button>"
                    )
                    return

                self.press_button(
                    args[1]
                )

            elif operation == "release":

                if len(args) != 2:
                    print(
                        "Usage: release <button>"
                    )
                    return

                self.release_button(
                    args[1]
                )

            elif operation == "button":

                if len(args) != 2:
                    print(
                        "Usage: button <button>"
                    )
                    return

                self.toggle_button(
                    args[1]
                )

            elif operation == "buttons":

                self.show_buttons()

            elif operation == "leds":

                self.show_leds()

            elif operation == "watch":

                if len(args) != 3:
                    print(
                        "Usage: watch "
                        "<gpio> <rising|falling|both>"
                    )
                    return

                gpio = int(args[1])
                edge = args[2].lower()

                self.watch_gpio(
                    gpio,
                    edge
                )

            elif operation == "unwatch":

                if len(args) != 2:
                    print(
                        "Usage: unwatch <gpio>"
                    )
                    return

                gpio = int(args[1])

                self.unwatch_gpio(
                    gpio
                )

            elif operation in (
                "quit",
                "exit"
            ):

                self.running = False

            else:

                print(
                    f"Unknown command: {operation}"
                )

                print(
                    "Type 'help' for commands"
                )

        except ValueError:

            print(
                "[ERROR] Invalid numeric value"
            )

        except Exception as exc:

            print(
                f"[ERROR] {exc}"
            )

    def run(self):

        self.print_help()

        print()
        print(
            "Simulator ready."
        )

        print(
            "Type 'help' for commands."
        )

        print()

        while self.running:

            try:

                command = input(
                    "stm32mp157-gpio> "
                )

                self.execute_command(
                    command
                )

            except KeyboardInterrupt:

                print()
                print(
                    "Ctrl+C received."
                )

                self.running = False

            except EOFError:

                print()
                self.running = False

        print()
        print(
            "STM32MP157-DK2 GPIO Simulator stopped."
        )


def parse_arguments():

    parser = argparse.ArgumentParser(
        description=(
            "STM32MP157-DK2 GPIO Simulator"
        )
    )

    parser.add_argument(
        "--config",
        default=None,
        help="GPIO simulator configuration JSON"
    )

    return parser.parse_args()


def main():

    args = parse_arguments()

    if args.config:

        config_file = args.config

    else:

        config_file = os.path.join(
            os.path.dirname(
                os.path.abspath(__file__)
            ),
            "config.json"
        )

    if not os.path.exists(config_file):

        print(
            f"[ERROR] Configuration file not found:"
            f" {config_file}"
        )

        return 1

    application = STM32GPIOApplication(
        config_file
    )

    application.run()

    return 0


if __name__ == "__main__":

    sys.exit(main())
```

---

# Directory after adding these files

Your simulator directory will now be:

```text
STM32MP157-DK2/
└── simulator/
    ├── config.json
    ├── gpio_events.py
    ├── gpio_model.py
    └── simulator.py
```

## How the files work together

The important architecture is:

```text
                  STM32MP157-DK2
                         |
                         |
                GPIO Simulator
                         |
                +--------+--------+
                |                 |
          simulator.py       config.json
                |
                v
          gpio_model.py
                |
        +-------+-------+
        |               |
    GPIOChip        GPIOLine
        |               |
        |        +------+------+------+
        |        |      |      |      |
        |      input  output  bias  value
        |
        v
   gpio_events.py
        |
   +----+-----+------+
   |          |      |
 Rising     Falling  Both
 Edge       Edge     Edge
   |
   v
Interrupt Callback
```

### Example: LED

If you execute:

```bash
gpio 1 1
```

the flow is:

```text
User command
    |
    v
simulator.py
    |
    v
model.set_gpio(1, 1)
    |
    v
GPIOChip.set_value()
    |
    v
GPIOLine.set_value()
    |
    v
GPIO1 = HIGH
    |
    v
_update_leds()
    |
    v
LED1 = ON
```

### Example: Button interrupt

Run:

```bash
watch 0 both
```

Then:

```bash
press BUTTON1
```

Flow:

```text
BUTTON1
   |
   v
GPIO0
   |
   v
simulate_input()
   |
   v
0 -> 1 / 1 -> 0
   |
   v
GPIOEvent
   |
   v
GPIOEventManager
   |
   v
interrupt_callback()
   |
   v
[INTERRUPT] GPIO0
```

This gives you a clean simulation layer that can later be connected to your actual **`gpio-libgpiod.c`**, **`gpio-sysfs.c`**, and **kernel `virtual_gpio.c`** implementation.

