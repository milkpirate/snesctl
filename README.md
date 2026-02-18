# SNESctl (`sˈnɛʃtəl`)
![GitHub License](https://img.shields.io/github/license/milkpirate/anycubic-buildroot)

<img src="https://upload.wikimedia.org/wikipedia/commons/5/5a/SNES-controller.png" alt="logo" height=200/>

Retrofit old SNES controllers with Bluetooth connectivity using the low power NRF52840 microcontroller.

This project uses the NRF52840 based [nice!nano] with the Adafruit
Bluefruit library for Bluetooth communication.

## Features
- Low power consumption with the NRF52840 microcontroller.
- Turns controller off between readings to save battery life.
- Compatible with most classic SNES controller replacements or knock offs.

## Hardware
The [nice!nano] has a built-in battery charger and can be powered by a single cell Li-ion battery. The SNES
controller is connected to the [nice!nano] using the 5 pin connector on the back of the controller. The 
[nice!nano] is quite sophisticated and the pins (of the employed SPI hardware) are almost freely configurable.
The controller can be connected to the microcontroller however it is convenient.

More details on the [nice!nano] and alternatives can be found in this repo: [nrfmicro].

## Software
The software is written in C++ using the Adafruits Arduino framework [Adafruit_nRF52_Arduino] via PlatformIO, with the 
[Nicenano-NRF52-Supermini-PlatformIO-Support] (sadly this one is a bit complicated to install, hence the `dummy` 
environment in the [`platformio.ini`]).

### Requirements
- [`task`](https://taskfile.dev/docs/installation) - modern `make` alternative for building and running tasks.
- [`uv`](https://docs.astral.sh/uv/getting-started/installation/) - modern Python environment and package manager.

### Building and flashing
```bash
task setup    # will will PlatformIO, require dependencies and the board definitions for nicenano 
task upload   # will build and flash the firmware to the device
```

## Documentation
For pinouts and timing diagrams please have a look at the [docs](./docs).

> NOTE: There are two switches to debug, namely `DEBUG_HID_REPORT` and `DEBUG_BATTERY_REPORT`. Please
> have a look at the [`platformio.ini`] for more details.

## Credits
_Image [SNES Controller](https://commons.wikimedia.org/wiki/File:SNES-controller.png) by Magnus Överengen, [CC0](https://creativecommons.org/publicdomain/zero/1.0/deed.en), via Wikimedia Commons_

[`platformio.ini`]: ./platformio.ini

[nice!nano]: https://nicekeyboards.com/nice-nano/
[nrfmicro]: https://github.com/joric/nrfmicro/wiki/Alternatives
[Adafruit_nRF52_Arduino]: https://github.com/adafruit/Adafruit_nRF52_Arduino
[Nicenano-NRF52-Supermini-PlatformIO-Support]: https://github.com/ICantMakeThings/Nicenano-NRF52-Supermini-PlatformIO-Support

