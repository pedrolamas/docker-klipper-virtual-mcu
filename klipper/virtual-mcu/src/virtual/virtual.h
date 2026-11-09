#ifndef __VIRTUAL_H
#define __VIRTUAL_H
// Shared state for the simulated "hardware" of the virtual mcu

#include <stdint.h> // uint32_t

// Pins use AVR style names (PA0..PL7) so existing simulavr configs work
#define VPORT_COUNT 12 // A..L
#define VGPIO(PORT, NUM) (((PORT) - 'A') * 8 + (NUM))
#define VPIN_COUNT (VPORT_COUNT * 8)

enum { VPIN_UNUSED, VPIN_OUT, VPIN_IN, VPIN_ADC, VPIN_PWM };

struct virtual_pin {
    uint8_t mode;      // VPIN_*
    uint8_t state;     // current output level (VPIN_OUT)
    int8_t pull_up;    // pull-up (>0) / pull-down (<0) for inputs
    uint16_t pwm;      // last hardware pwm value (VPIN_PWM)
    uint32_t changed;  // timer_read_time() of the last state change
};

struct virtual_pin *virtual_pin_get(uint32_t pin); // NULL if invalid

#endif // virtual.h
