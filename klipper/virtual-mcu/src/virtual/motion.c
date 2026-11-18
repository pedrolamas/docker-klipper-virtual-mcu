// Travel based endstops for the virtual mcu
//
// Each "axis" pairs a stepper's step and dir pins with an endstop pin.
// Steps are counted from the step pin (one step per edge away from its
// idle level, as Klipper pulses it) in the direction given by the dir
// pin, and the endstop pin reads as triggered while the carriage is at
// either end of the rail (position <= 0 or >= length). With switches
// at both ends the same axis works for configs that home to the minimum
// (cartesian, probes) and to the maximum (delta).
//
// Steppers in "step on both edges" mode (only enabled for TMC drivers)
// would be counted at half speed; TMC drivers are not simulated anyway.
//
// This file may be distributed under the terms of the GNU GPLv3 license.

#include <stdio.h> // fprintf
#include <stdlib.h> // strtod
#include <string.h> // strchr
#include "virtual.h" // struct virtual_pin

#define DEFAULT_STEPS_PER_MM 80.  // rotation_distance 40, 16 microsteps
#define DEFAULT_LENGTH_MM 300.
#define DEFAULT_START_MM 10.

struct motion_axis {
    struct virtual_pin *step, *dir, *endstop;
    uint8_t dir_invert, endstop_invert, step_idle;
    int32_t position, length; // in steps
};

#define MAX_AXES 8
static struct motion_axis axes[MAX_AXES];
static int axis_count;

// Default axes matching the shipped printer configs
static const char *default_axes[] = {
    "PB0:!PB1:PB2",     // stepper_x / stepper_a
    "PB3:!PC2:PC3",     // stepper_y / stepper_b
    "PC4:PC5:PC6",      // stepper_z / stepper_c (also the probe pin)
};


/****************************************************************
 * Pin hooks
 ****************************************************************/

// The step pin was (re)configured - val is its idle level
void
motion_step_setup(struct motion_axis *a, uint8_t val)
{
    a->step_idle = !!val;
}

// The step pin changed level
void
motion_step_changed(struct motion_axis *a, uint8_t val)
{
    if (val == a->step_idle)
        return;
    if (a->dir->state ^ a->dir_invert)
        a->position++;
    else
        a->position--;
}

uint8_t
motion_endstop_read(struct motion_axis *a)
{
    uint8_t triggered = a->position <= 0 || a->position >= a->length;
    return triggered ^ a->endstop_invert;
}


/****************************************************************
 * Setup
 ****************************************************************/

// Parse an optionally inverted ("!PB1") pin name
static int
parse_pin(char *name, struct virtual_pin **p, uint8_t *invert)
{
    *invert = name[0] == '!';
    uint32_t pin;
    if (virtual_pin_parse(name + *invert, &pin))
        return -1;
    *p = virtual_pin_get(pin);
    return 0;
}

// Parse the next optional ":NUMBER" field
static int
parse_number(char **pos, double *val)
{
    if (!*pos)
        return 0;
    char *field = *pos, *end;
    *pos = strchr(field, ':');
    if (*pos)
        *(*pos)++ = '\0';
    *val = strtod(field, &end);
    return end == field || *end || *val < 0. ? -1 : 0;
}

// Parse "STEP:[!]DIR:[!]ENDSTOP[:STEPS_PER_MM[:LENGTH_MM[:START_MM]]]"
// (eg, "PB0:!PB1:PB2" or "PB0:!PB1:PB2:80:300:10")
int
motion_add(const char *spec)
{
    char buf[96], *step = buf, *dir, *endstop, *rest;
    if (strlen(spec) >= sizeof(buf) || axis_count >= MAX_AXES)
        goto fail;
    strcpy(buf, spec);
    dir = strchr(step, ':');
    if (!dir)
        goto fail;
    *dir++ = '\0';
    endstop = strchr(dir, ':');
    if (!endstop)
        goto fail;
    *endstop++ = '\0';
    rest = strchr(endstop, ':');
    if (rest)
        *rest++ = '\0';

    struct motion_axis a = { };
    uint8_t step_invert;
    if (parse_pin(step, &a.step, &step_invert) || step_invert
        || parse_pin(dir, &a.dir, &a.dir_invert)
        || parse_pin(endstop, &a.endstop, &a.endstop_invert))
        goto fail;
    double steps_per_mm = DEFAULT_STEPS_PER_MM, length = DEFAULT_LENGTH_MM;
    double start = DEFAULT_START_MM;
    if (parse_number(&rest, &steps_per_mm) || parse_number(&rest, &length)
        || parse_number(&rest, &start) || rest)
        goto fail;
    a.length = length * steps_per_mm + .5;
    a.position = start * steps_per_mm + .5;
    if (steps_per_mm <= 0. || a.position <= 0 || a.position >= a.length)
        goto fail;
    if (a.step->moves || a.step == a.dir || a.step == a.endstop
        || a.dir == a.endstop || a.endstop->endstop)
        goto fail;

    struct motion_axis *ap = &axes[axis_count++];
    *ap = a;
    ap->step->moves = ap;
    ap->endstop->endstop = ap;
    fprintf(stderr, "Axis: step=%s dir=%s endstop=%s steps_per_mm=%.3f"
            " length=%.1fmm start=%.1fmm\n"
            , step, dir, endstop, steps_per_mm, length, start);
    return 0;
fail:
    fprintf(stderr, "Invalid axis '%s' (expected STEP_PIN:[!]DIR_PIN:"
            "[!]ENDSTOP_PIN[:STEPS_PER_MM[:LENGTH_MM[:START_MM]]],"
            " eg PB0:!PB1:PB2)\n", spec);
    return -1;
}

void
motion_add_defaults(void)
{
    int i;
    for (i = 0; i < sizeof(default_axes) / sizeof(default_axes[0]); i++)
        motion_add(default_axes[i]);
}
