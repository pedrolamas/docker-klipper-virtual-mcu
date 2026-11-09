// Simulated digital pins for the virtual mcu
//
// This file may be distributed under the terms of the GNU GPLv3 license.

#include "board/misc.h" // timer_read_time
#include "command.h" // shutdown
#include "gpio.h" // gpio_out_write
#include "sched.h" // sched_shutdown
#include "virtual.h" // struct virtual_pin

DECL_ENUMERATION_RANGE("pin", "PA0", VGPIO('A', 0), 8);
DECL_ENUMERATION_RANGE("pin", "PB0", VGPIO('B', 0), 8);
DECL_ENUMERATION_RANGE("pin", "PC0", VGPIO('C', 0), 8);
DECL_ENUMERATION_RANGE("pin", "PD0", VGPIO('D', 0), 8);
DECL_ENUMERATION_RANGE("pin", "PE0", VGPIO('E', 0), 8);
DECL_ENUMERATION_RANGE("pin", "PF0", VGPIO('F', 0), 8);
DECL_ENUMERATION_RANGE("pin", "PG0", VGPIO('G', 0), 8);
DECL_ENUMERATION_RANGE("pin", "PH0", VGPIO('H', 0), 8);
DECL_ENUMERATION_RANGE("pin", "PJ0", VGPIO('J', 0), 8);
DECL_ENUMERATION_RANGE("pin", "PK0", VGPIO('K', 0), 8);
DECL_ENUMERATION_RANGE("pin", "PL0", VGPIO('L', 0), 8);

static struct virtual_pin pins[VPIN_COUNT];

struct virtual_pin *
virtual_pin_get(uint32_t pin)
{
    if (pin >= VPIN_COUNT)
        return NULL;
    return &pins[pin];
}

// Parse an AVR style pin name ("PD7") into a pin id
int
virtual_pin_parse(const char *name, uint32_t *pin)
{
    if (name[0] != 'P' || name[1] < 'A' || name[1] > 'L' || name[1] == 'I'
        || name[2] < '0' || name[2] > '7' || name[3])
        return -1;
    *pin = VGPIO(name[1], name[2] - '0');
    return 0;
}

static void
pin_set_state(struct virtual_pin *p, uint8_t val)
{
    val = !!val;
    if (p->state != val) {
        uint32_t now = timer_read_time();
        if (p->heats)
            thermal_heater_changed(p->heats, p->state, now);
        p->state = val;
        p->changed = now;
    }
}

struct gpio_out
gpio_out_setup(uint32_t pin, uint8_t val)
{
    struct virtual_pin *p = virtual_pin_get(pin);
    if (!p)
        shutdown("Not an output pin");
    struct gpio_out g = { .pin = p };
    gpio_out_reset(g, val);
    return g;
}

void
gpio_out_reset(struct gpio_out g, uint8_t val)
{
    g.pin->mode = VPIN_OUT;
    pin_set_state(g.pin, val);
}

void
gpio_out_write(struct gpio_out g, uint8_t val)
{
    pin_set_state(g.pin, val);
}

void
gpio_out_toggle_noirq(struct gpio_out g)
{
    pin_set_state(g.pin, !g.pin->state);
}

void
gpio_out_toggle(struct gpio_out g)
{
    gpio_out_toggle_noirq(g);
}

struct gpio_in
gpio_in_setup(uint32_t pin, int8_t pull_up)
{
    struct virtual_pin *p = virtual_pin_get(pin);
    if (!p)
        shutdown("Not an input pin");
    struct gpio_in g = { .pin = p };
    gpio_in_reset(g, pull_up);
    return g;
}

void
gpio_in_reset(struct gpio_in g, int8_t pull_up)
{
    g.pin->mode = VPIN_IN;
    g.pin->pull_up = pull_up;
}

uint8_t
gpio_in_read(struct gpio_in g)
{
    // Nothing is wired to the pin, so it reads as its bias - the same
    // behaviour as an unconnected pin in simulavr.
    return g.pin->pull_up > 0;
}
