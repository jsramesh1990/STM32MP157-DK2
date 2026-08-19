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
