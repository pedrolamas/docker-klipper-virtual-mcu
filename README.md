# docker-klipper-virtual-mcu

[![Project Maintenance](https://img.shields.io/maintenance/yes/2026.svg)](https://github.com/pedrolamas/docker-klipper-virtual-mcu 'GitHub Repository')
[![License](https://img.shields.io/github/license/pedrolamas/docker-klipper-virtual-mcu.svg)](https://github.com/pedrolamas/docker-klipper-virtual-mcu/blob/master/LICENSE 'License')

[![Release](https://github.com/pedrolamas/docker-klipper-virtual-mcu/workflows/Release/badge.svg)](https://github.com/pedrolamas/docker-klipper-virtual-mcu/actions 'Build Status')

[![Follow pedrolamas.com on Bluesky](https://img.shields.io/badge/dynamic/json?url=https%3A%2F%2Fpublic.api.bsky.app%2Fxrpc%2Fapp.bsky.actor.getProfile%2F%3Factor%3Dpedrolamas.com&query=%24.followersCount&style=social&logo=bluesky&label=Follow%20%40pedrolamas.com)](https://bsky.app/profile/pedrolamas.com)
[![Follow pedrolamas on Mastodon](https://img.shields.io/mastodon/follow/109365776481898704?label=Follow%20@pedrolamas%20on%20Mastodon&domain=https%3A%2F%2Fhachyderm.io&style=social)](https://hachyderm.io/@pedrolamas)

Simple Docker image running [Klipper](https://github.com/Klipper3d/klipper/) with a simulated MCU, [Moonraker](https://github.com/Arksine/moonraker/), and [mjpg-streamer](https://github.com/jacksonliam/mjpg-streamer).

This repo will run a GitHub action every hour to check for new code on the default branches of the Klipper and Moonraker repositories, and creates a new Docker image if there are any modifications.

## Usage

Create and run the new container as you would normally do:

```sh
docker run -d \
  --name klipper-virtual-mcu \
  --net=host \
  ei99070/docker-klipper-virtual-mcu
```

This will start Klipper with a simulated micro-controller, Moonraker on port 7125, and mjpg-streamer on port 8080.

If you need to remap the default ports, run the container under the bridge network instead:

```sh
docker run -d \
  --name klipper-virtual-mcu \
  -p 7125:7125 \
  -p 8080:8080 \
  ei99070/docker-klipper-virtual-mcu
```

## Virtual MCU

The printer's main MCU is a native "virtual MCU": the Klipper micro-controller code compiled as a Linux process with simulated pins, exposing its serial port at `/tmp/pseudoserial`. It uses almost no CPU when idle and keeps accurate time.

> [!NOTE]
> This image replaces [docker-klipper-simulavr](https://github.com/pedrolamas/docker-klipper-simulavr), which used [simulavr](https://github.com/pedrolamas/simulavr) to emulate an ATmega644P. The bundled configuration uses the same pins, and the `SIMULAVR_PACING_RATE` environment variable is no longer needed.

| Variable           | Default | Description                                   |
| ------------------ | ------- | --------------------------------------------- |
| `VIRTUAL_MCU_ARGS` |         | Extra options for the virtual MCU (see below) |

### Simulated heaters

The virtual MCU simulates heaters: each "thermal zone" pairs a heater pin with a thermistor (EPCOS 100K B57560G104F, 4.7K pull-up) sensor pin, and the temperature follows the heater's duty cycle. The default zones match the bundled configuration:

| Heater | Heater pin | Sensor pin | Preset   |
| ------ | ---------- | ---------- | -------- |
| `extruder`   | `PD7` | `PA5` | `hotend` (~200°C in 40s, 300°C max) |
| `extruder1`  | `PD3` | `PA6` | `hotend` |
| `heater_bed` | `PB4` | `PA7` | `bed` (~60°C in 150s, 130°C max) |

If you use your own `printer.cfg` with different pins, replace the zones with `-T HEATER_PIN:SENSOR_PIN:PRESET` options (repeatable, or `-T none` to disable them), for example `-e VIRTUAL_MCU_ARGS="-T PD5:PA0:hotend -T PD6:PA1:bed"`. Analog pins without a zone read a constant 25°C.

### Simulated endstops and probe

The virtual MCU also tracks where each axis is: it counts the steps sent to the stepper's step pin, in the direction set by its dir pin, and the endstop pin reads as triggered while the carriage is at either end of a 300mm rail. Each carriage starts 10mm from the minimum end, so homing travels a real distance, with the usual retract and second approach, whether the config homes to the minimum (cartesian) or to the maximum (delta). The default axes match the bundled configuration:

| Axis                    | Step pin | Dir pin | Endstop pin |
| ----------------------- | -------- | ------- | ----------- |
| `stepper_x`/`stepper_a` | `PB0`    | `!PB1`  | `PB2`       |
| `stepper_y`/`stepper_b` | `PB3`    | `!PC2`  | `PC3`       |
| `stepper_z`/`stepper_c` | `PC4`    | `PC5`   | `PC6`       |

`PC6` is also the `[probe]` pin, which triggers at Z=0, so `PROBE`, `PROBE_ACCURACY` and `BED_MESH_CALIBRATE` work (the simulated bed is perfectly flat). BLTouch is not simulated.

If you use your own `printer.cfg` with different pins, replace the axes with `-E STEP_PIN:DIR_PIN:ENDSTOP_PIN[:STEPS_PER_MM[:LENGTH_MM[:START_MM]]]` options (repeatable, or `-E none` to disable them). Prefix the dir pin with `!` if it is inverted in the config, and the endstop pin with `!` if it is inverted (triggered when low). The defaults are 80 steps/mm (`rotation_distance: 40` with 16 microsteps), a 300mm rail and a 10mm start position, for example `-e VIRTUAL_MCU_ARGS="-E PB0:!PB1:PB2 -E PB3:PC2:!PC3:160"`. Input pins without an axis read as their pull-up.

### Using configs from other boards

To make configs written for real boards easier to reuse, the virtual MCU accepts:

- AVR and STM32 style pin names, `PA0`..`PL15`, and RP2040 style pin names, `gpio0`..`gpio47`
- common SPI bus names (such as `spi1`, `spi2a` and `spi0a`) and I2C bus names (such as `i2c1`, `i2c2a` and `i2c0a`), which all reach the same empty bus: writes succeed and reads return zeros

Heaters, thermistors, endstops and the probe on other pins still need matching `-T` and `-E` options. Devices that must answer the MCU, such as TMC stepper drivers and accelerometers, are not simulated, so remove those sections from the config.

The default configuration files used can be found on the [klipper_config](/klipper_config) folder.

This is the runtime folder structure:

```txt
/printer
  /klipper
  /klippy-env
  /mjpg-streamer
  /moonraker
  /moonraker-env
  /printer_data
    /config
      /moonraker.conf
      /printer.cfg
    /database
    /gcodes
    /logs
      /klippy.log
      /moonraker.log
      /supervisord.log
    /moonraker.asvc
```

Any of these files can be overrided by mapping the folder or the specific file.

For example, here is how to override the default `printer.cfg`:

```sh
docker run -d \
  --name klipper-virtual-mcu \
  --net=host \
  -v my-printer.cfg:/printer/printer_data/config/printer.cfg \
  ei99070/docker-klipper-virtual-mcu
```

## Klippy Extras

Some Klipper extra modules are included as part of this image, specifically:

- `virtual_pins` - https://github.com/pedrolamas/klipper-virtual-pins
- `gcode_shell_command` - https://github.com/th33xitus/kiauh

## Convenience scripts

The image includes the following convenience scripts:

- `enable-timelapse` - installs the Moonraker Timelapse dependencies, updates the configuration files, and restart Klipper and Moonraker
- `restore-klipper-repo` - restores the Klipper git repository to the same point where the docker image was created
- `restore-moonraker-repo` - restores the Moonraker git repository to the same point where the docker image was created

Once the Docker container has started, these can be easily run:

```sh
docker exec -it klipper-virtual-mcu enable-timelapse
```

## Available tags

- `latest`: points to Klipper and Moonraker default branches
- `klipper-sha-<hash>`: points to the Klipper GitHub commit hash
- `moonraker-sha-<hash>`: points to the Moonraker GitHub commit hash

## Support my work

A lot of time and effort goes into the development of this and other open-source projects.

If you find this project valuable, please consider supporting my work by making a donation.

[![Donate on Paypal](https://img.shields.io/badge/donate-paypal-blue.svg)](https://paypal.me/pedrolamas 'Donate on Paypal')
[![Buy me a coffee](https://img.shields.io/badge/buy%20me%20a%20coffee-kofi-blue.svg)](https://ko-fi.com/pedrolamas 'Buy me a coffee')
[![Support me on Patreon](https://img.shields.io/badge/join-patreon-blue.svg)](https://www.patreon.com/pedrolamas 'Support me on Patreon')
[![Sponsor me on GitHub](https://img.shields.io/github/sponsors/pedrolamas.svg?label=github%20sponsors)](https://github.com/sponsors/pedrolamas 'Sponsor me on GitHub')

Thank you for your generosity and support! 🙏

## License

MIT
