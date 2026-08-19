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
