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
