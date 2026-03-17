# 💡 7Driver

A 4-digit 7-segment LED display driver built on a Saturday years ago, using an ATmega328P Arduino and a couple of spare TTL logic chips (74XX595 and 74XX155). Designed as part of a larger bench ecosystem, this module emphasizes classic hardware logic, multiplexing, and minimal pin usage via serial interfacing.

> If you found this project useful, interesting, or worth keeping an eye on, consider giving it a ⭐️.
> It helps others discover the project and motivates me to keep building and sharing more.

## 🔹 Construction

![7Driver 1](<Construction/7Driver 1.jpg>)

## 🔹 Rev 1 Schematic

![Rev 1](<Schematics/Rev 1.png>)

## 🔹 V1.0.0 Firmware For Rev 1 Schematic

- Initial release.

## 🔹 Key Features

- Displays numbers on a 4-digit 7-segment LED display.
- Built using:
  - 74XX595 – shift register (serial data output).
  - 74XX155 – 2-to-4 line decoder/demux.
- Controlled by an ATmega328P.
- Segment timing and animation handled in software.
- Basic Power-On Self-Test (POST) on startup.
- Tested with various counting logic.
- ADC input (with potentiometer) for interactive control or display testing.

## 🔹 Design Notes

- RCLK (Pin 12) of the 74LS595 is tied to SRCLK (Pin 11), so outputs update automatically with each bit shifted in, which was useful for early animation tests.

- Designed to minimize microcontroller pin usage:
  - Serial output via 74LS595 for segment data.
  - Digit selection via 74LS155 demux.

- Early versions featured:
  - Simple animations.
  - Shift speed control using an ADC-connected potentiometer.

## 🔹 General Usage

- Performs a Power-On Self-Test (POST) to verify segment connections and logic IC behavior.
- Best used with a regulated 5V supply.

## 🔹 Considered Improvements

- Break out RCLK from SRCLK for controlled output updates.
- Add individual resistors per LED segment for more consistent brightness.
- Extend character support beyond 0–9 and A–F (e.g., letters, symbols).
- Logic to limit counting to prevent overflow or undefined patterns, so it doesn’t lose its digits. ☺️

##

> Educational Use Notice: This project is provided for educational and learning purposes only. You are welcome to read, study, and experiment
> with this software and/or hardware. It is not intended for commercial use. This software and/or hardware is provided "as is", without warranty
> of any kind. The author assumes no responsibility for any damages or issues resulting from its use.