// Simulated hardware pwm for the virtual mcu
//
// This file may be distributed under the terms of the GNU GPLv3 license.

#include "board/misc.h" // timer_read_time
#include "command.h" // DECL_CONSTANT
#include "gpio.h" // gpio_pwm_write
#include "sched.h" // sched_shutdown
#include "virtual.h" // virtual_pin_get

DECL_CONSTANT("PWM_MAX", 255);

struct gpio_pwm
gpio_pwm_setup(uint32_t pin, uint32_t cycle_time, uint16_t val)
{
    struct virtual_pin *p = virtual_pin_get(pin);
    if (!p)
        shutdown("Not a valid PWM pin");
    p->mode = VPIN_PWM;
    struct gpio_pwm g = { .pin = p };
    gpio_pwm_write(g, val);
    return g;
}

void
gpio_pwm_write(struct gpio_pwm g, uint16_t val)
{
    if (g.pin->pwm != val) {
        g.pin->pwm = val;
        g.pin->changed = timer_read_time();
    }
}
