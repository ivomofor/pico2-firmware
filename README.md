# Raspberry Pi Pico 2 W Firmware Project

## Overview

This project is part of my embedded C and firmware development learning journey.

The goal is to develop practical firmware for the Raspberry Pi Pico 2 W, progressing from basic microcontroller programming to GPIO control, peripherals, communication protocols, and hardware interfacing.

The firmware currently initializes USB standard I/O and repeatedly prints a message over USB serial.

## Hardware

- **Board:** Raspberry Pi Pico 2 W
- **Microcontroller:** RP2350
- **Processor:** ARM Cortex-M33
- **Development computer:** Ubuntu Linux

## Development Environment

The project uses:

- Raspberry Pi Pico SDK 2.3.1
- ARM GNU Toolchain 13.2.1
- GCC cross-compiler
- CMake
- GNU Make
- Git

The Pico SDK is located at:

```text
~/project/pico-sdk
```

The `PICO_SDK_PATH` environment variable points to this directory.

## Project Structure

```text
pico2-firmware/
─ CMakeLists.txt
─ README.md
─ main.c
─ .gitignore
─ build/
```

The `build/` directory contains generated build files and firmware artifacts. It is excluded from Git.

## Building the Firmware

Configure the project:

```bash
cmake -S . -B build
```

Compile the firmware:

```bash
cmake --build build -j4
```

The build generates several firmware artifacts, including:

```text
build/pico2_firmware.elf
build/pico2_firmware.uf2
build/pico2_firmware.bin
build/pico2_firmware.hex
```

The UF2 file is used to flash the Raspberry Pi Pico 2 W.

## Flashing the Firmware

1. Disconnect the Pico 2 W from USB.
2. Hold down the BOOTSEL button.
3. While holding BOOTSEL, connect the board to the computer.
4. Release BOOTSEL.
5. Confirm that the RP2350 bootloader appears as a USB storage device.
6. Copy the firmware:

```bash
cp build/pico2_firmware.uf2 /media/$USER/RP2350/
sync
```

The Pico automatically restarts after flashing.

## Viewing USB Serial Output

The firmware uses USB CDC serial communication.

Connect to the Pico using:

```bash
screen /dev/ttyACM0 115200
```

Expected output:

```text
Hello from Raspberry Pi Pico 2 W!
Hello from Raspberry Pi Pico 2 W!
Hello from Raspberry Pi Pico 2 W!
```

The message is printed approximately once per second.

To exit the Screen session:

1. Press `Ctrl+A`.
2. Press `\`.
3. Press `Y` to confirm.

Exiting Screen does not stop the firmware running on the Pico.

## Current Firmware Features

- USB standard I/O initialization.
- USB CDC serial communication.
- Continuous execution using an infinite loop.
- One second delay using `sleep_ms()`.
- Repeated output using `printf()`.

## Future Development

Planned topics include:

- GPIO input and output.
- Onboard LED control using the CYW43 interface.
- Buttons and external LEDs.
- Timers and interrupts.
- UART communication.
- I2C and SPI.
- Sensor interfacing.
- Firmware debugging and testing.

## Git

Check the current repository status:

```bash
git status
```
