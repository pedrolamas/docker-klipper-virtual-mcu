// Simulated analog inputs for the virtual mcu
//
// This file may be distributed under the terms of the GNU GPLv3 license.

#include "command.h" // DECL_CONSTANT
#include "gpio.h" // gpio_adc_setup
#include "sched.h" // sched_shutdown
#include "virtual.h" // virtual_pin_get

// Same resolution as the AVR adc the bundled configs were written for
#define ADC_MAX 1023
DECL_CONSTANT("ADC_MAX", ADC_MAX);

// 100K NTC thermistor at 25C with a 4.7K pull-up: 100/(100+4.7) of full
// scale. Reported on analog pins that have no thermal zone attached.
#define ADC_ROOM_TEMP ((uint16_t)(ADC_MAX * 100000ULL / (100000 + 4700)))

struct gpio_adc
gpio_adc_setup(uint32_t pin)
{
    struct virtual_pin *p = virtual_pin_get(pin);
    if (!p)
        shutdown("Not a valid ADC pin");
    p->mode = VPIN_ADC;
    return (struct gpio_adc){ .pin = p };
}

uint32_t
gpio_adc_sample(struct gpio_adc g)
{
    // Sample is always ready
    return 0;
}

uint16_t
gpio_adc_read(struct gpio_adc g)
{
    if (g.pin->senses)
        return thermal_adc_read(g.pin->senses, ADC_MAX);
    return ADC_ROOM_TEMP;
}

void
gpio_adc_cancel_sample(struct gpio_adc g)
{
}
