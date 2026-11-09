# Virtual MCU Pinmap

This document provides a comprehensive pinmap for the virtual MCU and how its pins are configured in the bundled configuration files. The pin names and numbering follow the ATmega644P layout the configuration was originally written for.

## Pin Configuration Table

| Pin # | Pin Code | Pin Function             | Klipper Usage            |
|-------|----------|--------------------------|--------------------------|
| 1     | PB0      | T0/XCK0                  | stepper_x_step_pin       |
| 2     | PB1      | T1                       | stepper_x_dir_pin        |
| 3     | PB2      | AIN0/INT2                | stepper_x_endstop_pin    |
| 4     | PB3      | AIN1/OC0A                | stepper_y_step_pin       |
| 5     | PB4      | SS/OC0B                  | heater_bed_heater_pin    |
| 6     | PB5      | MOSI                     | -                        |
| 7     | PB6      | MISO                     | -                        |
| 8     | PB7      | SCK                      | -                        |
| 9     | RESET    | Reset (Active Low)       | -                        |
| 10    | GND      | Ground                   | -                        |
| 11    | VCC      | +5V Power                | -                        |
| 12    | XTAL2    | Crystal Oscillator Pin 2 | -                        |
| 13    | XTAL1    | Crystal Oscillator Pin 1 | -                        |
| 14    | PC0      | SCL                      | -                        |
| 15    | PC1      | SDA                      | -                        |
| 16    | PC2      | TCK                      | stepper_y_dir_pin        |
| 17    | PC3      | TMS                      | stepper_y_endstop_pin    |
| 18    | PC4      | TDO                      | stepper_z_step_pin       |
| 19    | PC5      | TDI                      | stepper_z_dir_pin        |
| 20    | PC6      | TOSC1                    | stepper_z_endstop_pin    |
| 21    | PC7      | TOSC2                    | extruder_step_pin        |
| 22    | PD0      | RXD0                     | -                        |
| 23    | PD1      | TXD0                     | -                        |
| 24    | PD2      | RXD1/INT0                | -                        |
| 25    | PD3      | TXD1/INT1                | extruder1_heater_pin     |
| 26    | PD4      | OC1B                     | extruder_dir_pin         |
| 27    | PD5      | OC1A                     | extruder1_step_pin       |
| 28    | PD6      | OC2B/ICP1                | extruder1_dir_pin        |
| 29    | PD7      | OC2A                     | extruder_heater_pin      |
| 30    | GND      | Analog Ground            | -                        |
| 31    | AVCC     | Analog VCC               | -                        |
| 32    | AREF     | Analog Reference         | -                        |
| 33    | PA7      | ADC7                     | heater_bed_sensor_pin    |
| 34    | PA6      | ADC6                     | extruder1_sensor_pin     |
| 35    | PA5      | ADC5                     | extruder_sensor_pin      |
| 36    | PA4      | ADC4                     | -                        |
| 37    | PA3      | ADC3                     | -                        |
| 38    | PA2      | ADC2                     | -                        |
| 39    | PA1      | ADC1                     | -                        |
| 40    | PA0      | ADC0                     | -                        |

## Pin Usage Summary

### Stepper Motors

- **X-Axis Stepper**: PB0 (step), PB1 (direction), PB2 (endstop)
- **Y-Axis Stepper**: PB3 (step), PC2 (direction), PC3 (endstop)
- **Z-Axis Stepper**: PC4 (step), PC5 (direction), PC6 (endstop)

### Extruders

- **Primary Extruder**: PC7 (step), PD4 (direction), PD7 (heater), PA5 (sensor)
- **Secondary Extruder**: PD5 (step), PD6 (direction), PD3 (heater), PA6 (sensor)

### Heaters and Temperature Sensors

- **Heated Bed**: PB4 (heater), PA7 (sensor)
- **Extruder**: PD7 (heater), PA5 (sensor)
- **Extruder1**: PD3 (heater), PA6 (sensor)

Each heater/sensor pair is a simulated thermal zone, so temperatures respond to the heater.

### Available Pins

The following pins are currently unassigned and available for additional features:

- PA0, PA1, PA2, PA3, PA4 (ADC channels)
- PB5, PB6, PB7 (SPI interface)
- PC0, PC1 (I²C/TWI interface)
- PD0, PD1 (UART0)
- PD2 (UART1)

### Communication Interfaces

- **SPI**: PB4 (SS), PB5 (MOSI), PB6 (MISO), PB7 (SCK) - PB4 used by the bed heater
- **I²C/TWI**: PC0 (SCL), PC1 (SDA) - Available
- **UART0**: PD0 (RX), PD1 (TX) - Available
- **UART1**: PD2 (RX), PD3 (TX) - PD3 used by the extruder1 heater

## Notes

- This pinmap is designed for the **virtual MCU** with the Klipper firmware
- The pin assignments are defined in the Klipper configuration files for virtual hardware simulation
- Fans and enable pins use virtual pins rather than physical pins; heaters use MCU pins
- ADC channels (Port A) are used for temperature sensing simulation
- JTAG debugging pins (PC2-PC6) and timer-related pins (PB0-PB1, PC6-PC7, PD4-PD6) are repurposed for stepper motor control
- All hardware communication interfaces (SPI, I²C, UART) remain available for expansion
- The virtual MCU accepts any pin from PA0 to PL7; only the ATmega644P ports (A-D) are listed here
