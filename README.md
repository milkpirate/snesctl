# SNESctl (`sˈnɛʃtəl`)
[![GitHub License](https://img.shields.io/github/license/milkpirate/snesctl)](./LICENSE)
[![build](https://github.com/milkpirate/snesctl/actions/workflows/build.yml/badge.svg)](https://github.com/milkpirate/snesctl/actions/workflows/build.yml)

<img src="docs/snes-controller.png" alt="logo" height="200"/>

Retrofit old SNES controllers with Bluetooth connectivity using the low power NRF52840 microcontroller.

This project uses the NRF52840 based [nice!nano] with the Adafruit
Bluefruit library for Bluetooth communication.

## Features
- Low power consumption with the NRF52840 microcontroller.
- Turns the controller off between readings to save battery life.
- Powers down completely if no connection is made within 2 minutes, so a forgotten controller does not drain
  the battery.
- Reports the battery level via the standard BLE battery service.
- Compatible with most classic SNES controllers, replacements and knock-offs.

## Hardware
The [nice!nano] has a built-in battery charger and can be powered by a single cell Li-ion battery. The SNES
controller is connected to the [nice!nano] using the 5 wires of the controller cable. The [nice!nano] is quite
sophisticated and the pins (of the employed SPI hardware) are almost freely configurable, so the controller can be
connected to the microcontroller however it is convenient.

More details on the [nice!nano] and alternatives can be found in this repo: [nrfmicro].

### Parts
- [nice!nano] v2 or v1 (or a compatible clone, e.g. the "SuperMini nRF52840"), see [board revisions](#board-revisions)
- SNES controller (original or knock-off)
- single cell LiPo battery that fits into the controller housing
- power switch between the battery and the `BATTERY+` pin (to power cycle the controller)

> NOTE: The charge current of the [nice!nano] is 100 mA by default. Only bridge the `BOOST` jumper (500 mA) if the
> battery is larger than 500 mAh.

### Wiring
The pins are configured in [`include/Config.hpp`](./include/Config.hpp) (`Config::Pins`). The wire colors are the ones of
the controller used during development and differ between manufacturers, so check the controller's PCB.

| SNES controller | nice!nano pin | Wire color (example) | Notes                                                   |
|-----------------|---------------|----------------------|---------------------------------------------------------|
| VCC             | `3.3V`        | white                | switched on/off via `P0.13` (external VCC cutoff)       |
| Clock           | `P0.31`       | blue                 | SPI SCK                                                 |
| Latch           | `P0.29`       | yellow               |                                                         |
| Data            | `P0.02`       | red                  | SPI MISO                                                |
| GND             | `P1.15`       | brown                | driven low while reading, floating in between           |

`P1.13` is claimed as SPI MOSI by the firmware, but unused (configured as input), so leave it unconnected.

See the [docs](./docs) for the pinouts of the [nice!nano] ([v2](./docs/pinout-v2.png), [v1](./docs/pinout.png)).

### Board revisions
There is a firmware for each board, as they differ in (configured in `config/<board>/BoardConfig.hpp`):

|                               | nice!nano v1                      | nice!nano v2      | SuperMini nRF52840           |
|-------------------------------|-----------------------------------|-------------------|------------------------------|
| PlatformIO environment        | `nicenano_v1`                     | `nicenano_v2`     | `supermini`                  |
| external VCC switch (`P0.13`) | active low                        | active high       | active high                  |
| battery voltage measurement   | voltage divider on `P0.04` (AIN2) | internal VDDH/5   | internal VDDH/5              |
| internal DC/DC converter      | off (not verified)                | off (not verified)| on                           |

The [SuperMini nRF52840](https://github.com/joric/nrfmicro/wiki/Alternatives#supermini-nrf52840) (aka "ProMicro
NRF52840") is a cheap nice!nano v2 clone with some quirks: early batches leak ~700 uA through the pull-up of the
external VCC switch while it is switched off (check with `task vcc-test`), some have a leaky diode (D1, should be a
BAT60B Schottky) and a faulty 32.768 kHz crystal (connection drops). Power cycle it after flashing, to reach the low
sleep current.

The revision is printed on the board (`v1.0`/`v2.0`, next to the nice!nano logo). Clones usually behave like the
v2. If unsure, flash the probe firmware with `task probe` and watch its output with `task monitor`: it checks how
the chip is supplied (v2: battery on VDDH, "high voltage mode") and prints the result every second. Afterwards,
flash the actual firmware for the identified revision.

### Button mapping
The controller shows up as a standard BLE HID gamepad. It reads the controller right before every Bluetooth
connection event (every 11.25-15 ms, depending on the host, similar to the ~60 Hz of the SNES) and sends the state in
that very event, if it changed:

| SNES button | HID gamepad |
|-------------|-------------|
| B           | button 0    |
| Y           | button 1    |
| Select      | button 2    |
| Start       | button 3    |
| A           | button 4    |
| X           | button 5    |
| L           | button 6    |
| R           | button 7    |
| D-pad       | X/Y axis    |

## Usage
1. Switch the controller on, it starts advertising as `SNESctl`.
2. Pair it in the Bluetooth settings of your phone/laptop/console within 2 minutes.
3. Open any application that accepts gamepad input.

The status LED flashes while waiting for a connection and is off otherwise. To save the battery
(see `Config::Power` in [`include/Config.hpp`](./include/Config.hpp)):

- If no connection is made within 2 minutes (also after a disconnect), the controller powers down completely
  (nRF52 System OFF). Power cycle it with the switch (or plug in USB) to start over.
- If no button is pressed for 15 minutes while connected, it disconnects and goes to light sleep. Press any button
  to wake it up (it reconnects to the last device). After 60 minutes in light sleep, it powers down completely.

The battery level is reported via the standard BLE battery service (shown e.g. in the Bluetooth settings), based on
a typical LiPo discharge curve (see `Config::Battery::lipo_curve` in [`include/Config.hpp`](./include/Config.hpp)).
While USB is plugged in, the board measures the USB supply instead of the battery, so it reports ~100%.

## Software
The software is written in C++ using Adafruit's Arduino framework [Adafruit_nRF52_Arduino] via PlatformIO. The
board definition and pinout (variant) of the nice!nano are downloaded from
[Nicenano-NRF52-Supermini-PlatformIO-Support] by `task setup` (pinned to a commit, updated by Renovate), so
nothing has to be installed into PlatformIO manually.

### Requirements
- [`task`](https://taskfile.dev/docs/installation) - modern `make` alternative for building and running tasks.
- [`uv`](https://docs.astral.sh/uv/getting-started/installation/) - modern Python environment and package manager.
- a host C++ compiler (e.g. `gcc`), for running the unit tests only.

### Building and flashing
```bash
task setup    # installs PlatformIO, the required platforms, toolchains, frameworks and the nice!nano board
task run      # builds the firmware (.hex, .zip and .uf2) for both board revisions
task upload   # builds and flashes the firmware for the SuperMini to the device via USB
task upload BOARD=nicenano_v2  # ...for the nice!nano v2 (or nicenano_v1)
```

Alternatively, flash the `firmware.uf2` (from `.pio/build/<board>/`, or the artifacts of the
[build pipeline](https://github.com/milkpirate/snesctl/actions/workflows/build.yml)) by drag and drop: double tap
the reset button of the [nice!nano], so it shows up as USB drive, and copy the file onto it.

The firmware revision (`git describe`) is reported via the BLE device information service.

### Other tasks
```bash
task test       # runs the unit tests (Catch2) on the host
task probe      # flashes a probe firmware that identifies the nice!nano revision (see Board revisions)
task vcc-test   # flashes a test firmware to measure the leak of the VCC switch pull-up (see tools/vcc-test)
task coverage   # ...and creates a coverage report (reports/c.cov.html)
task monitor    # opens the serial monitor
task debug      # builds a debug firmware and starts a GDB session (needs a J-Link, also takes BOARD=...)
task clean      # removes the build artifacts
task clean:all  # removes everything generated, incl. PlatformIO and the Python environment
```

Pass additional PlatformIO arguments after `--`, e.g. `task upload -- --upload-port COM5`.

### Dependency updates
Dependencies (GitHub Actions, Python packages, Catch2 and the nice!nano board files) are kept up to date by
[Renovate](https://docs.renovatebot.com) (see [`renovate.json`](./renovate.json)): nightly, with minor and patch
updates merged automatically once the pipeline passes. It can also be run manually via the
[renovate workflow](https://github.com/milkpirate/snesctl/actions/workflows/renovate.yml), which authenticates
with a GitHub App of this repository, see the [setup](./docs/renovate.md).

## Documentation
For pinouts and timing diagrams please have a look at the [docs](./docs).

> NOTE: There are some debug switches, namely `WAIT_FOR_SERIAL`, `DEBUG_HID_REPORT` and `DEBUG_BATTERY_REPORT`.
> Please have a look at the [`platformio.ini`] for more details.

## Credits
_Image [SNES Controller](https://commons.wikimedia.org/wiki/File:SNES-controller.png) by Magnus Överengen, [CC0](https://creativecommons.org/publicdomain/zero/1.0/deed.en), via Wikimedia Commons_

_nice!nano board definition and variant from [Nicenano-NRF52-Supermini-PlatformIO-Support]_

[`platformio.ini`]: ./platformio.ini

[nice!nano]: https://nicekeyboards.com/nice-nano/
[nrfmicro]: https://github.com/joric/nrfmicro/wiki/Alternatives
[Adafruit_nRF52_Arduino]: https://github.com/adafruit/Adafruit_nRF52_Arduino
[Nicenano-NRF52-Supermini-PlatformIO-Support]: https://github.com/ICantMakeThings/Nicenano-NRF52-Supermini-PlatformIO-Support
